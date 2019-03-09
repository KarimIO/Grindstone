#include <iostream>
#include "FastNoise.hpp"
#include "ErosionNode.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#include <fstream>

int main() {
	int seed = 237;

	FastNoise noise(300);
	int width, height = width = 512;
	double *data = new double[width * height];

	double scale = 1.0;

	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			data[i * height + j] = noise.GetPerlin(j * scale, i * scale) * 0.5 + 0.5;
		}
	}

	ErosionNode e;
	e.setMap(data, width, height);
	e.setMaxDropletLifetime(80);
	e.setSedimentCapacityFactor(10);
	double *eroded = e.calculate();
	unsigned char *img1 = new unsigned char[width * height];
	unsigned char *img2 = new unsigned char[width * height];
	unsigned short *img3 = new unsigned short[width * height];

	for (int i = 0; i < height * width; ++i) {
		img1[i] = 255 * data[i];
		img2[i] = 255 * eroded[i];
		img3[i] = 65535 * eroded[i];
	}

	stbi_write_png("land1.png", width, height, 1, img1, width);
	stbi_write_png("land2.png", width, height, 1, img2, width);

	std::ofstream out("land3.raw");
	out.write((const char*)img2, width * height * sizeof(unsigned char));
	out.close();

	//system("pause");
	return 0;
}