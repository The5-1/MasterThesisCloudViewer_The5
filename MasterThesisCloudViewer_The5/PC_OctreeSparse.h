#pragma once
//Extern
#include <glm\glm.hpp>
#include <GL/glew.h>
#include <GL/glut.h>
#include "helper.h"
//Std
#include <iostream>
#include <vector>
#include <bitset>

struct OctreeBox {
public:
	std::bitset<8> bitMaskChildren; // [0,0,0,0,0,0,0,0], set to 1 if the corresponding child exists
	std::vector<OctreeBox> children;
	std::vector<unsigned int> indexList;

	OctreeBox() {

	}
};


class PC_OctreeSparse
{
	//Variables
public:
	OctreeBox root;


	//Functions
public:
	PC_OctreeSparse(std::vector<glm::vec3>& _vertices, std::vector<glm::vec3>& _colors);

	//createGrid(int )
};

