#pragma once

#include <cstdio>

#include "glm.hpp"

struct Triangle
{
	glm::vec3 v[3];

	Triangle();
	Triangle(const glm::vec3&, const glm::vec3&, const glm::vec3&);
	Triangle& operator= (const Triangle& tr);
};
