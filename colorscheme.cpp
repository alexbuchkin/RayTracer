#include "colorscheme.hpp"

#include <cmath>

glm::vec3
getColorByValue(float value, float maxValue)
{
	float red = value / maxValue * 255.0f;
	float blue = 255.0f - red;
	return glm::vec3(red, 0.0f, blue);
}