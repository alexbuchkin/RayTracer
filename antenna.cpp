#include "antenna.hpp"

#include "gtc/random.hpp"

#include <stdexcept>

Antenna::Antenna(const glm::vec3& origin, float radius, float power):
	origin(origin),
	power(power),
	radius(radius)
{
	if (radius <= 0.0f) {
		throw std::invalid_argument("Radius must be positive");
	}
}

Antenna::Antenna(const Antenna& antenna):
	origin(antenna.origin),
	power(antenna.power),
	radius(antenna.radius)
	{}

glm::vec3
Antenna::getPosition() const noexcept
{
	return origin;
}

float
Antenna::getRadius() const noexcept
{
	return radius;
}

float
Antenna::getPower() const noexcept
{
	return power;
}

WifiRay
Antenna::emitRandomRay() const
{
	glm::vec3 direction = glm::sphericalRand(radius);
	return WifiRay(origin, glm::normalize(direction), power);
}