all:
	g++ main.cpp scene.cpp auxstructures.cpp wifiray.cpp antenna.cpp tracer.cpp camera.cpp colorscheme.cpp -o exec -std=c++11 -I lib -I lib/glm -fopenmp

clean:
	rm exec
