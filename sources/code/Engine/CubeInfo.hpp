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
	{ "ft", glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f, 0.0f) },
	{ "bk", glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f, 0.0f) },
	{ "up", glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f, 1.0f) },
	{ "dn", glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f,-1.0f) },
	{ "rt", glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f, 0.0f) },
	{ "lf", glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f) }
};

#endif