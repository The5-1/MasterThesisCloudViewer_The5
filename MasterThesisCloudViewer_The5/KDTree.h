#pragma once
#include <vector>
#include <glm\glm.hpp>

#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

class KDTree {
private:
	std::vector<int> indices, indicesTemp;

public:
	KDTree(std::vector<glm::vec3> vertices);

	
};