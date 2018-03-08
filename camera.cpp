#include "camera.hpp"

#include "gtx/intersect.hpp"
#include "gtx/normal.hpp"
#include "EasyBMP.hpp"

#include <cmath>
#include <cstdio>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <string>
#include <iostream>

Camera::Camera(const Scene& scene,
	const glm::vec3& pos,
	const glm::vec3& viewDir,
	const glm::vec3& up,
	const glm::vec3& right,
	float heightAngle,
	float widthAngle,
	int leastDim
	):
	scene(scene),
	pos(pos),
	viewDir(glm::normalize(viewDir)),
	up(glm::normalize(up)),
	right(glm::normalize(right))
{
	if (leastDim <= 0) {
		throw(std::invalid_argument("leastDim must be positive"));
	}
	if (heightAngle <= 0.0f) {
		throw(std::invalid_argument("heightAngle must be positive"));
	}
	if (heightAngle >= float(M_PI)) {
		throw(std::invalid_argument("heightAngle must be less than PI"));
	}
	if (widthAngle <= 0.0f) {
		throw(std::invalid_argument("widthAngle must be positive"));
	}
	if (widthAngle >= float(M_PI)) {
		throw(std::invalid_argument("widthAngle must be less than PI"));
	}
	if (leastDim % 2 != 0) ++leastDim;

	if (heightAngle < widthAngle) {
		dimH = leastDim;
		height = 2.0f * tan(heightAngle / 2.0f);
		float pixelSide = height / float(leastDim);
		width = 2.0f * tan(widthAngle / 2.0f);
		dimW = int(ceil(width / pixelSide));
		width = float(dimW) * pixelSide;
	} else {
		dimW = leastDim;
		width = 2.0f * tan(widthAngle / 2.0f);
		float pixelSide = width / float(leastDim);
		height = 2.0f * tan(heightAngle / 2.0f);
		dimH = int(ceil(height / pixelSide));
		height = float(dimH) * pixelSide;
	}
}

WifiRay
Camera::emitRayThroughPixel(int h, int w)
{
	if (h < 0 || h > dimH) {
		throw(std::invalid_argument("incorrect height"));
	}
	if (w < 0 || w > dimW) {
		throw(std::invalid_argument("incorrect width"));
	}

	float x = (width  / float(dimW)) * (float(w - dimW / 2) + 0.5f);
	float y = (height / float(dimH)) * (float(dimH / 2 - h) - 0.5f);

	return WifiRay(pos, glm::normalize(viewDir + x * right + y * up));
}

glm::vec3
Camera::getPixelColor(int h, int w)
{
	WifiRay ray = emitRayThroughPixel(h, w);

	bool intersection = false;
	Triangle tr;
	float distance = 0.0f;
	for (int i = 0; i < scene.numberOfMeshes(); ++i) {
		//ignoring roof
		float eps1 = scene.getMaxZ() - scene[i].v[0].z;
		float eps2 = scene.getMaxZ() - scene[i].v[1].z;
		float eps3 = scene.getMaxZ() - scene[i].v[2].z;
		if (eps1 < 0.0001f && eps2 < 0.0001f && eps3 < 0.0001f) {
			continue;
		}
		//checking intersection
		std::pair<bool, float> ret = ray.checkIntersection(scene[i]);
		if (ret.first == true) {
			if (intersection == false || distance > ret.second) {
				intersection = true;
				tr = scene[i];
				distance = ret.second;
			}
		}
	}

	//check if ray intersects sphere
	bool sphereIsCloser = false;//true if ray intersects sphere before any triangle
	glm::vec3 intersectionPoint;
	glm::vec3 intersectionNormal;

	if (glm::intersectRaySphere(ray.getCoord(),
								ray.getDirection(),
								scene.antenna.getPosition(),
								scene.antenna.getRadius(),
								intersectionPoint,
								intersectionNormal))
	{
		float dist2 = glm::distance(ray.getCoord(), intersectionPoint);
		//if sphere is closer to pixel than any mesh
		if (intersection == false || dist2 < distance) {
			intersection = true;
			sphereIsCloser = true;
			distance = dist2;
		}
	}

	//case if no intersection
	//checking that ray intersects voxel grid
	/*
	bool borderIntersection = false;
	if (intersection == false) {
		for (int i = 0; i < 12; ++i) {
			//checking intersection
			std::pair<bool, float> ret = ray.checkIntersection(scene.getBorderTriangle(i));
			if (ret.first == true) {
				if (borderIntersection == false || distance < ret.second) {
					borderIntersection = true;
					tr = scene.getBorderTriangle(i);
					distance = ret.second;
				}
			}
		}

		if (borderIntersection == false) {//no ray's dot belongs to voxel grid
			return glm::vec3(0.0f, 0.0f, 0.0f);
		}
	}
	*/
	if (intersection == false) {
		return glm::vec3(0.0f, 0.0f, 0.0f);//black
	}

	intersectionPoint = ray.getCoord() + distance * ray.getDirection();

	WifiRay backRay(intersectionPoint, glm::normalize(ray.getCoord() - intersectionPoint));
	glm::vec3 color(0.0f, 0.0f, 0.0f);
	glm::vec3 n;

	if (sphereIsCloser) {
		n = intersectionNormal;
	} else {
		n = glm::triangleNormal(tr.v[0], tr.v[1], tr.v[2]);
	}

	float lightIntensity = glm::dot(backRay.getDirection(), glm::normalize(n)) * 255.0f;
	if (lightIntensity < 0.0f) lightIntensity *= -1.0f;
	color.x = lightIntensity;
	color.y = lightIntensity;
	color.z = lightIntensity;






	//back tracing
	glm::vec3 voxelSize = scene.getVoxelSize();
	float stepSize = std::min(std::min(voxelSize.x, voxelSize.y), voxelSize.z) / 2.0f;
	//std::cout << "StepSize = " << stepSize << std::endl;
	float alpha = 0.02f;

	while (!scene.inBounds(backRay.getCoord())) {
		backRay.makeStep(0.01f);
	}

	while (scene.inBounds(backRay.getCoord())) {
		float value = scene.getVoxelValue(backRay.getCoord());
		if (value >= std::min(1.0f, scene.antenna.power / 1000.0f)) {
		//if (value > 0.0f) {
			glm::vec3 newColor = getColorByValue(value, scene.antenna.power);
			color = newColor * alpha + color * (1.0f - alpha);
		}
		backRay.makeStep(stepSize);
	}

	return color;
}








void
Camera::takePhoto(const char* path)
{
	using uint = unsigned int;
	EasyBMP::Image image(dimW, dimH);
	int h, w;
	#pragma omp parallel for private(h, w) collapse(2)
	for (h = 0; h < dimH; ++h) {
		for (w = 0; w < dimW; ++w) {
			glm::vec3 color = getPixelColor(h, w);
			uint r = std::min(uint(255), uint(round(color.x)));
			uint g = std::min(uint(255), uint(round(color.y)));
			uint b = std::min(uint(255), uint(round(color.z)));

			image.SetPixel(w, h, EasyBMP::RGBColor(r, g, b));
		}
	}

	image.Write(std::string(path));
}