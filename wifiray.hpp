#pragma once

#include "glm.hpp"

#include "auxstructures.hpp"

#include <utility>

class WifiRay
{
	glm::vec3 origin;
	glm::vec3 direction;
	float traveledDistance;
	const float antennaPower;

	//reflection parameters
	int reflectionTimes;
	bool isReflected;//true if intersection point is set
	glm::vec3 intersectionPoint;
	glm::vec3 reflectDirection;


public:
	WifiRay(const glm::vec3& origin, const glm::vec3& direction, float power = 1000.0f);
	float getPower() const;
	glm::vec3 getCoord() const;
	glm::vec3 getDirection() const;
	bool makeStep(float stepSize);
	std::pair<bool, float> checkIntersection(const Triangle& tr) const;//if first is true then second is distance
	void setReflection(const Triangle& tr);
	int getReflectionTimes() const;

	//debug
	float getTraveledDistance() const {return traveledDistance;}
	bool reflected() const {return isReflected;}
};