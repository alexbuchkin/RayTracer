#include "wifiray.hpp"

#include "gtx/intersect.hpp"
#include "gtx/normal.hpp"

WifiRay::WifiRay(const glm::vec3& origin, const glm::vec3& direction, float power):
					origin(origin),
					direction(glm::normalize(direction)),
					antennaPower(power),
					traveledDistance(0.0f),
					isReflected(false),
					reflectionTimes(0)
					{}

float
WifiRay::getPower() const
{
	return antennaPower - traveledDistance;
}

glm::vec3
WifiRay::getCoord() const
{
	return origin;
}

glm::vec3
WifiRay::getDirection() const
{
	return direction;
}

int
WifiRay::getReflectionTimes() const
{
	return reflectionTimes;
}

bool
WifiRay::makeStep(float stepSize)
{
	bool achieved = false;
	float dist = glm::distance(origin, intersectionPoint);

	if (isReflected && dist <= stepSize) {//ray achieved intersection point
		++reflectionTimes;
		achieved = true;
		origin = intersectionPoint;
		direction = reflectDirection;
		traveledDistance += dist;
		isReflected = false;//because new reflection point is unknown
	} else {
		origin += direction * stepSize;
		traveledDistance += stepSize;
	}
	return achieved;
}

std::pair <bool, float>//if intersects then (true, dist) else (false, undefined)
WifiRay::checkIntersection(const Triangle& tr) const
{
	glm::vec3 baryPos;
	bool gotIntersect = glm::intersectRayTriangle(
		origin,
		direction,
		tr.v[0],
		tr.v[1],
		tr.v[2],
		baryPos
	);

	if (gotIntersect == false) {
		return std::make_pair(false, 0.0f);
	} else {
		return std::make_pair(true, baryPos.z);
	}
}

void
WifiRay::setReflection(const Triangle& tr)//only if intersection is true
{
	isReflected = true;

	glm::vec3 baryPos;
	glm::intersectRayTriangle(origin,
							  direction,
							  tr.v[0],
							  tr.v[1],
							  tr.v[2],
							  baryPos);

	intersectionPoint = origin + direction * baryPos.z;

	glm::vec3 trNormal = glm::normalize(glm::triangleNormal(tr.v[0], tr.v[1], tr.v[2]));

	reflectDirection = glm::normalize(glm::reflect(direction, trNormal));
}