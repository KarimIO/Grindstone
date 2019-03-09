#pragma once

#include <vector>

class PerlinNoiseGenerator {
public:
	PerlinNoiseGenerator();
	PerlinNoiseGenerator(int seed);
	double getNoiseAtPoint();
};