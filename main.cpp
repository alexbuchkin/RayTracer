#define TINYOBJLOADER_IMPLEMENTATION

#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdio>
#include <cmath>
#include "tiny_obj_loader.h"
#include "glm.hpp"
#include "gtx/intersect.hpp"
#include "gtx/normal.hpp"

#include "scene.hpp"
#include "tracer.hpp"
#include "camera.hpp"

int
main()
{
	glm::vec3 antennaPosition(10000.0f, 2000.0f, 100.0f);
	Antenna antenna(antennaPosition, 1000.0f, 100000.0f);

	Scene scene(antenna, 200, 200, 20);
	scene.parseObjFile("rooms/Flat.obj");

	std::cout << "Preparing..." << std::endl;

	Tracer tracer(scene, 7);
	int i;
	#pragma omp parallel for private(i)
	for (i = 0; i < 10000; ++i) {
		tracer.traceWifiRay();
	}

	std::cout << "Applying box filter..." << std::endl;

	scene.applyBoxFilter();

	glm::vec3 pos(13000.0f, 1000.0f, 10000.0f);
	glm::vec3 viewDir(0.0f, 0.0f, -1.0f);
	glm::vec3 up(0.0f, 1.0f, 0.0f);
	glm::vec3 right(1.0f, 0.0f, 0.0f);

	std::cout << "Taking photo..." << std::endl;

	Camera camera(scene, pos, viewDir, up, right, M_PI / 2.0, M_PI / 2.0, 1024);

	camera.takePhoto("photo.bmp");

	return 0;
}
