#include "tracer.hpp"

#include <stdexcept>
#include <utility>
//
#include <iostream>
#include <cstdio>
//

Tracer::Tracer(Scene& scene, int maxReflectionTimes):
	scene(scene),
	maxReflectionTimes(maxReflectionTimes)
	{}

void
Tracer::setReflection(WifiRay& ray) const
{
	bool isIntersect = false;
	float dist = 0.0f;//distance to nearest triangle
	Triangle tr;
	std::pair<bool, float> ret;

	for (int i = 0; i < scene.numberOfMeshes(); ++i) {
		ret = ray.checkIntersection(scene[i]);//ret is pair<bool, float>
		if (ret.first == true) {
			if (ret.second < 0.001f) continue;//to avoid choosing triangle ray origin belongs to
			if (isIntersect == false/*first triangle ray intersects with*/ || dist > ret.second/*current triangle is closer than prev*/) {
				isIntersect = true;
				dist = ret.second;
				tr = scene[i];
			}
		}
	}

	if (isIntersect == true) {
		ray.setReflection(tr);
	}
}

void
Tracer::traceWifiRay()
{
	glm::vec3 size = scene.getVoxelSize();
	const float stepSize = std::min(size.x, std::min(size.y, size.z)) / 10.0f;

	WifiRay ray = scene.antenna.emitRandomRay();
	setReflection(ray);

	while (ray.getPower() > std::min(1.0f, scene.antenna.getPower() / 10000.0f) && scene.inBounds(ray.getCoord())) {
		scene.updateVoxel(ray.getCoord(), ray.getPower());

		bool b = ray.makeStep(stepSize);//b is true if ray reflected at this step
		
		if (b == true) {
			if (maxReflectionTimes < 0 || ray.getReflectionTimes() <= maxReflectionTimes) {
				setReflection(ray);
			} else {
				return;
			}
		}
	}
}