#ifndef _CUBE_INFO
#define _CUBE_INFO

struct CubemapDirection
{
	char name[3];
	glm::vec3 Target;
	glm::vec3 Up;
};

const CubemapDirection gCubeDirections[6] =
{
	{ "FT", glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f, 0.0f) },
	{ "BK", glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f, 0.0f) },
	{ "UP", glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f, 1.0f) },
	{ "DN", glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f,-1.0f) },
	{ "RT", glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f, 0.0f) },
	{ "LF", glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f) }
};

#endif