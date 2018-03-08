#pragma once

#include "scene.hpp"
#include "wifiray.hpp"
#include "antenna.hpp"
#include "auxstructures.hpp"

#include "glm.hpp"

class Tracer
{
	Scene& scene;
	int maxReflectionTimes;

	void setReflection(WifiRay& ray) const;

public:
	Tracer(Scene& scene, int maxReflectionTimes = 0);
	void traceWifiRay();
};