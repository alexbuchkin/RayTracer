#pragma once

#include "glm.hpp"

#include "wifiray.hpp"

class Antenna
{

public:
	const glm::vec3 origin;
	const float radius;
	const float power;

	Antenna(const glm::vec3& origin, float radius = 1.0f, float power = 1000.0f);
	Antenna(const Antenna& antenna);
	glm::vec3 getPosition() const noexcept;
	float getRadius() const noexcept;
	float getPower() const noexcept;

	WifiRay emitRandomRay() const;//emits ray from its center
};