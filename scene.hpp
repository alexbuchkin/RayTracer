#pragma once

#include <vector>
#include <string>
#include <stdexcept>
#include <tuple>

#include "glm.hpp"

#include "antenna.hpp"
#include "auxstructures.hpp"

class Scene
{
	std::vector <std::vector <std::vector <float>>> voxelGrid;
	const int gridX;
	const int gridY;
	const int gridZ;
	std::vector<Triangle> triangles;
	glm::vec3 minCoords;
	glm::vec3 maxCoords;
	std::vector<Triangle> borderTriangles;//border parallelepiped will be divided into triangles and stored here

	float& getVoxel(const glm::vec3& dot);//get access to voxel containing given dot
	const float& getVoxel(const glm::vec3& dot) const;

public:
	const Antenna antenna;

	Scene(const Antenna& antenna,
		  int gridX = 100,
		  int gridY = 100,
		  int gridZ = 100
		  );

	void parseObjFile(const char* path);
	void applyBoxFilter(int radius = 1);
	bool inBounds(const glm::vec3& dot) const;//check if dot is inside grid
	glm::vec3 getVoxelSize() const;
	void updateVoxel(const glm::vec3& dot, float value);//if voxel value is less than given then update it
	float getVoxelValue(const glm::vec3& dot) const;

	int numberOfMeshes() const;//just returns triangles.size()
	const Triangle& operator[](int i) const;//access to triangles
	const Triangle& getBorderTriangle(int i) const;//access to border triangles

	float getMaxZ() const noexcept;//for ignoring roof
};