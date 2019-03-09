#include "ErosionNode.hpp"
#include <cmath>
#include <cstring>

unsigned int randInt(unsigned int min, unsigned int max) {
	return rand() % (max - min) + min;
}

ErosionNode::ErosionNode() {
}

void ErosionNode::setSeed(int seed) {
	srand(seed);
}

void ErosionNode::setErosionRadius(int radius) {
	if (radius >= 2 && radius <= 8)
		erosionRadius = radius;
}

void ErosionNode::setInertia(float i) {
	if (i >= 0.0f && i <= 1.0f)
		inertia = i;
}

void ErosionNode::setSedimentCapacityFactor(float capacity) {
	sedimentCapacityFactor = capacity;
}

void ErosionNode::setMinSedimentCapacity(float capacity) {
	minSedimentCapacity = capacity;
}

void ErosionNode::setErodeSpeed(float speed) {
	if (speed >= 0.0f && speed <= 1.0f)
		erodeSpeed = speed;
}

void ErosionNode::setDepositSpeed(float speed) {
	if (speed >= 0.0f && speed <= 1.0f)
		depositSpeed = speed;
}

void ErosionNode::setEvaporateSpeed(float speed) {
	if (speed >= 0.0f && speed <= 1.0f)
		evaporateSpeed = speed;
}

void ErosionNode::setGravity(float g) {
	gravity = g;
}

void ErosionNode::setMaxDropletLifetime(int life) {
	maxDropletLifetime = life;
}

void ErosionNode::setInitialWaterVolume(float water) {
	initialWaterVolume = water;
}

void ErosionNode::setInitialSpeed(float speed) {
	initialSpeed = speed;
}

void ErosionNode::setMap(double *source, unsigned int w, unsigned int h) {
	width = w;
	height = h;
	map = new double[width * height];
	memcpy(map, source, width * height * sizeof(double));

	initializeBrushIndices();
}

double * ErosionNode::calculate() {
	erode(50000);
	return map;
}

void ErosionNode::erode(int numIterations) {
	for (int iteration = 0; iteration < numIterations; iteration++) {
		// Create water droplet at random point on map
		float posX = randInt(0, width - 1);
		float posY = randInt(0, height - 1);
		float dirX = 0;
		float dirY = 0;
		float speed = initialSpeed;
		float water = initialWaterVolume;
		float sediment = 0;

		for (int lifetime = 0; lifetime < maxDropletLifetime; lifetime++) {
			int nodeX = (int)posX;
			int nodeY = (int)posY;
			int dropletIndex = nodeY * width + nodeX;
			// Calculate droplet's offset inside the cell (0,0) = at NW node, (1,1) = at SE node
			float cellOffsetX = posX - nodeX;
			float cellOffsetY = posY - nodeY;

			// Calculate droplet's height and direction of flow with bilinear interpolation of surrounding heights
			HeightAndGradient heightAndGradient = calculateHeightAndGradient(map, height, posX, posY);

			// Update the droplet's direction and position (move position 1 unit regardless of speed)
			dirX = (dirX * inertia - heightAndGradient.gradientX * (1 - inertia));
			dirY = (dirY * inertia - heightAndGradient.gradientY * (1 - inertia));
			// Normalize direction
			float len = sqrt(dirX * dirX + dirY * dirY);
			if (len != 0) {
				dirX /= len;
				dirY /= len;
			}
			posX += dirX;
			posY += dirY;

			// Stop simulating droplet if it's not moving or has flowed over edge of map
			if ((dirX == 0 && dirY == 0) || posX < 0 || posX >= width - 1 || posY < 0 || posY >= height - 1) {
				break;
			}

			// Find the droplet's new height and calculate the deltaHeight
			float newHeight = calculateHeightAndGradient(map, width, posX, posY).height;
			float deltaHeight = newHeight - heightAndGradient.height;

			// Calculate the droplet's sediment capacity (higher when moving fast down a slope and contains lots of water)
			float sedimentCapacity = std::fmax(-deltaHeight * speed * water * sedimentCapacityFactor, minSedimentCapacity);

			// If carrying more sediment than capacity, or if flowing uphill:
			if (sediment > sedimentCapacity || deltaHeight > 0) {
				// If moving uphill (deltaHeight > 0) try fill up to the current height, otherwise deposit a fraction of the excess sediment
				float amountToDeposit = (deltaHeight > 0) ? std::fmin(deltaHeight, sediment) : (sediment - sedimentCapacity) * depositSpeed;
				sediment -= amountToDeposit;

				// Add the sediment to the four nodes of the current cell using bilinear interpolation
				// Deposition is not distributed over a radius (like erosion) so that it can fill small pits
				map[dropletIndex] += amountToDeposit * (1 - cellOffsetX) * (1 - cellOffsetY);
				map[dropletIndex + 1] += amountToDeposit * cellOffsetX * (1 - cellOffsetY);
				map[dropletIndex + width] += amountToDeposit * (1 - cellOffsetX) * cellOffsetY;
				map[dropletIndex + width + 1] += amountToDeposit * cellOffsetX * cellOffsetY;

			}
			else {
				// Erode a fraction of the droplet's current carry capacity.
				// Clamp the erosion to the change in height so that it doesn't dig a hole in the terrain behind the droplet
				float amountToErode = std::fmin((sedimentCapacity - sediment) * erodeSpeed, -deltaHeight);

				// Use erosion brush to erode from all nodes inside the droplet's erosion radius
				for (int brushPointIndex = 0; brushPointIndex < erosionBrushIndicesLengths[dropletIndex]; brushPointIndex++) {
					int nodeIndex = erosionBrushIndices[dropletIndex][brushPointIndex];
					float weighedErodeAmount = amountToErode * erosionBrushWeights[dropletIndex][brushPointIndex];
					float deltaSediment = (map[nodeIndex] < weighedErodeAmount) ? map[nodeIndex] : weighedErodeAmount;
					map[nodeIndex] -= deltaSediment;
					sediment += deltaSediment;
				}
			}

			// Update droplet's speed and water content
			speed = std::sqrt(speed * speed + deltaHeight * gravity);
			water *= (1 - evaporateSpeed);
		}
	}
}

ErosionNode::HeightAndGradient ErosionNode::calculateHeightAndGradient(double *nodes, int mapSize, float posX, float posY) {
	int coordX = (int)posX;
	int coordY = (int)posY;

	// Calculate droplet's offset inside the cell (0,0) = at NW node, (1,1) = at SE node
	float x = posX - coordX;
	float y = posY - coordY;

	// Calculate heights of the four nodes of the droplet's cell
	int nodeIndexNW = coordY * mapSize + coordX;
	float heightNW = nodes[nodeIndexNW];
	float heightNE = nodes[nodeIndexNW + 1];
	float heightSW = nodes[nodeIndexNW + mapSize];
	float heightSE = nodes[nodeIndexNW + mapSize + 1];

	// Calculate droplet's direction of flow with bilinear interpolation of height difference along the edges
	float gradientX = (heightNE - heightNW) * (1 - y) + (heightSE - heightSW) * y;
	float gradientY = (heightSW - heightNW) * (1 - x) + (heightSE - heightNE) * x;

	// Calculate height with bilinear interpolation of the heights of the nodes of the cell
	float height = heightNW * (1 - x) * (1 - y) + heightNE * x * (1 - y) + heightSW * (1 - x) * y + heightSE * x * y;

	return { height = height, gradientX = gradientX, gradientY = gradientY };
}

void ErosionNode::initializeBrushIndices() {
	erosionBrushIndicesLengths = new int[width * height];
	erosionBrushIndices = new int*[width * height];
	erosionBrushWeights = new float*[width * height];

	int radius = erosionRadius;
	int mapSize = height;

	int *xOffsets = new int[radius * radius * 4];
	int *yOffsets = new int[radius * radius * 4];
	float *weights = new float[radius * radius * 4];
	float weightSum = 0;
	int addIndex = 0;

	for (int i = 0; i < width * height; i++) {
		int centreX = i % mapSize;
		int centreY = i / mapSize;

		if (centreY <= radius || centreY >= mapSize - radius || centreX <= radius + 1 || centreX >= mapSize - radius) {
			weightSum = 0;
			addIndex = 0;
			for (int y = -radius; y <= radius; y++) {
				for (int x = -radius; x <= radius; x++) {
					float sqrDst = x * x + y * y;
					if (sqrDst < radius * radius) {
						int coordX = centreX + x;
						int coordY = centreY + y;

						if (coordX >= 0 && coordX < mapSize && coordY >= 0 && coordY < mapSize) {
							float weight = 1 - sqrt(sqrDst) / radius;
							weightSum += weight;
							weights[addIndex] = weight;
							xOffsets[addIndex] = x;
							yOffsets[addIndex] = y;
							addIndex++;
						}
					}
				}
			}
		}

		int numEntries = addIndex;
		erosionBrushIndicesLengths[i] = numEntries;
		erosionBrushIndices[i] = new int[numEntries];
		erosionBrushWeights[i] = new float[numEntries];

		for (int j = 0; j < numEntries; j++) {
			erosionBrushIndices[i][j] = (yOffsets[j] + centreY) * mapSize + xOffsets[j] + centreX;
			erosionBrushWeights[i][j] = weights[j] / weightSum;
		}
	}

	delete xOffsets;
	delete yOffsets;
	delete weights;
}
