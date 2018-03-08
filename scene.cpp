#include "scene.hpp"
#include "tiny_obj_loader.h"

#include <stdexcept>
#include <cmath>
#include <set>

Scene::Scene(const Antenna& antenna, int gridX, int gridY, int gridZ):
	antenna(antenna),
	gridX(gridX),
	gridY(gridY),
	gridZ(gridZ)
{
	voxelGrid.resize(gridX);
	for (int i = 0; i < gridX; ++i) {
		voxelGrid[i].resize(gridY);
		for (int j = 0; j < gridY; ++j) {
			voxelGrid[i][j].resize(gridZ, 0.0f);
		}
	}
}

bool
Scene::inBounds(const glm::vec3& dot) const
{
	return (dot.x >= minCoords.x && dot.x <= maxCoords.x &&
			dot.y >= minCoords.y && dot.y <= maxCoords.y &&
			dot.z >= minCoords.z && dot.z <= maxCoords.z);
}

float&
Scene::getVoxel(const glm::vec3& dot)
{
	if (!inBounds(dot)) {
		throw std::invalid_argument("Dot is out of bounds");
	}

	int x = float(floor((dot.x - minCoords.x) / (maxCoords.x - minCoords.x) * float(gridX)));
	if (x == gridX) --x;
	int y = float(floor((dot.y - minCoords.y) / (maxCoords.y - minCoords.y) * float(gridY)));
	while (y < 0) ++y;
	while (y >= gridY) --y;
	int z = float(floor((dot.z - minCoords.z) / (maxCoords.z - minCoords.z) * float(gridZ)));
	while (z < 0) ++z;
	while (z >= gridZ) --z;

	return voxelGrid[x][y][z];
}

const float&
Scene::getVoxel(const glm::vec3& dot) const
{
	if (!inBounds(dot)) {
		throw std::invalid_argument("Dot is out of bounds");
	}

	int x = float(floor((dot.x - minCoords.x) / (maxCoords.x - minCoords.x) * float(gridX)));
	if (x == gridX) --x;
	int y = float(floor((dot.y - minCoords.y) / (maxCoords.y - minCoords.y) * float(gridY)));
	if (y == gridY) --y;
	int z = float(floor((dot.z - minCoords.z) / (maxCoords.z - minCoords.z) * float(gridZ)));
	if (z == gridZ) --z;

	return voxelGrid[x][y][z];
}

void
Scene::parseObjFile(const char* path)
{
	//opening file
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;

	bool isOpened = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path, NULL, true);
	if (!isOpened) {
		throw std::invalid_argument(err);
	}

	//collecting vertices from attrib
	std::vector<glm::vec3> vertices;
	vertices.reserve(attrib.vertices.size() / 3);

	for (int i = 0; i < attrib.vertices.size(); i += 3) {
		float x = float(attrib.vertices[i]);
		float y = float(attrib.vertices[i + 1]);
		float z = float(attrib.vertices[i + 2]);

		vertices.push_back(glm::vec3(x, y, z));
	}

	//collecting triangle meshes
	for (const auto& currentShape : shapes) {
		for (int i = 0; i < currentShape.mesh.indices.size(); i += 3) {
			int ix = currentShape.mesh.indices[i].vertex_index;
			int iy = currentShape.mesh.indices[i + 1].vertex_index;
			int iz = currentShape.mesh.indices[i + 2].vertex_index;

			triangles.push_back(Triangle(vertices[ix], vertices[iy], vertices[iz]));
		}
	}

	//finding bounds
	minCoords = vertices[0];
	maxCoords = vertices[0];

	for (const auto& i : vertices) {
		minCoords = glm::min(minCoords, i);
		maxCoords = glm::max(maxCoords, i);
	}

	const float eps = 0.0001f;
	const glm::vec3 epsVec(eps, eps, eps);
	minCoords -= epsVec;
	maxCoords += epsVec;

	//setting border triangles
	borderTriangles.reserve(12);
	glm::vec3 d[2][2][2];
	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < 2; ++j) {
			for (int k = 0; k < 2; ++k) {
				d[i][j][k] = glm::vec3(
					i ? maxCoords.x : minCoords.x,
					j ? maxCoords.y : minCoords.y,
					k ? maxCoords.z : minCoords.z
					);
			}
		}
	}
	//x
	borderTriangles.push_back(Triangle(d[0][0][0], d[0][0][1], d[0][1][0]));
	borderTriangles.push_back(Triangle(d[0][1][1], d[0][0][1], d[0][1][0]));
	borderTriangles.push_back(Triangle(d[1][0][0], d[1][0][1], d[1][1][0]));
	borderTriangles.push_back(Triangle(d[1][1][1], d[1][0][1], d[1][1][0]));
	//y
	borderTriangles.push_back(Triangle(d[0][0][0], d[0][0][1], d[1][0][0]));
	borderTriangles.push_back(Triangle(d[1][0][1], d[0][0][1], d[1][0][0]));
	borderTriangles.push_back(Triangle(d[0][1][0], d[0][1][1], d[1][1][0]));
	borderTriangles.push_back(Triangle(d[1][1][1], d[0][1][1], d[1][1][0]));
	//z
	borderTriangles.push_back(Triangle(d[0][0][0], d[0][1][0], d[1][0][0]));
	borderTriangles.push_back(Triangle(d[1][1][0], d[0][1][0], d[1][0][0]));
	borderTriangles.push_back(Triangle(d[0][0][1], d[0][1][1], d[1][0][1]));
	borderTriangles.push_back(Triangle(d[1][1][1], d[0][1][1], d[1][0][1]));
}

void
Scene::applyBoxFilter(int radius)
{
	if (radius <= 0) {
		throw std::invalid_argument("Radius must be positive");
	}

	for (int i = radius; i < gridX - radius; ++i)
	for (int j = radius; j < gridY - radius; ++j)
	for (int k = radius; k < gridZ - radius; ++k)
	{
		float sum = 0.0f;
		for (int ii = i - radius; ii <= i + radius; ++ii)
		for (int jj = j - radius; jj <= j + radius; ++jj)
		for (int kk = k - radius; kk <= k + radius; ++kk)
		{
			sum += voxelGrid[ii][jj][kk];
		}
		sum /= float((2 * radius + 1) * (2 * radius + 1) * (2 * radius + 1));
		voxelGrid[i][j][k] = sum;
	}
}

glm::vec3
Scene::getVoxelSize() const
{
	glm::vec3 v;
	v.x = (maxCoords.x - minCoords.x) / float(gridX);
	v.y = (maxCoords.y - minCoords.y) / float(gridY);
	v.z = (maxCoords.z - minCoords.z) / float(gridZ);

	return v;
}

void
Scene::updateVoxel(const glm::vec3& dot, float value)
{
	float& voxel = getVoxel(dot);
	voxel = std::max(voxel, value);
}

float
Scene::getVoxelValue(const glm::vec3& dot) const
{
	return getVoxel(dot);
}

int
Scene::numberOfMeshes() const
{
	return triangles.size();
}

const Triangle&
Scene::operator[](int i) const
{
	return triangles.at(i);
}

const Triangle&
Scene::getBorderTriangle(int i) const
{
	return triangles.at(i);
}

float
Scene::getMaxZ() const noexcept
{
	return maxCoords.z;
}
