#pragma once

#include "glm.hpp"

#include "scene.hpp"
#include "wifiray.hpp"
#include "colorscheme.hpp"

#include <vector>
#include <string>

class Camera
{
	const Scene& scene;
	glm::vec3 pos;
	glm::vec3 viewDir;//is normalized
	glm::vec3 up;
	glm::vec3 right;
	int dimH;//number of pixels in one column
	int dimW;//number of pixels in one row
	float height;//height of picture
	float width;//width of picture

	int rays = 0;

	WifiRay emitRayThroughPixel(int h, int w);
	glm::vec3 getPixelColor(int h, int w);

public:
	Camera(const Scene& scene,
		   const glm::vec3& pos,
		   const glm::vec3& viewDir,
		   const glm::vec3& up,
		   const glm::vec3& right,
		   float heightAngle,
		   float widthAngle,
		   int leastDim = 512//smallest side must have at least (leastDim) pixels
		   );
	void takePhoto(const char* path = "photos/photo1.bmp");
};