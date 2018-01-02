#pragma once
#include <fstream>
#include <iterator>
#include <algorithm>
#include <iostream>

#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <GL/glew.h>
#include <GL/glut.h>

#include <bitset>
#include <vector>

struct OctreeBoxViewer {
public:
	unsigned long int numPoints = 0;
	unsigned char bitMaskChar = 0;
	glm::vec3 minLeafBox, maxLeafBox;

	std::vector<OctreeBoxViewer> childs;
public:

	OctreeBoxViewer()
	{
		//std::cout << "childs size: " << childs.size() << std::endl;
		//childs = std::vector<OctreeBox>(0);
		//std::cout << "childs size: " << childs.size() << std::endl;
	}

	//~OctreeBoxViewer() = default;
};

class PC_Viewer
{
	//Variables
public:
	OctreeBoxViewer root;

	std::string pathFolder; //"D:/Dev/Assets/Pointcloud/ATL_RGB_vehicle_scan-20171228T203225Z-001/ATL_RGB_vehicle_scan/Potree"

	int points = 723386;

	double boundingBoxMinX = 2240519.12;
	double boundingBoxMinY = 1363315.304;
	double boundingBoxMinZ = 1028.255;

	double boundingBoxMaxX = 2240951.213;
	double boundingBoxMaxY = 1363747.3969999999;
	double boundingBoxMaxZ = 1460.3479999998772;

	double spacing = 3.742035150527954;	 //3.742035150527954,
	float scale = 0.01f;	//	0.01,
	int hierarchyStepSize = 5; // 5

	//GL Point Cloud
	GLuint vboPC[2];
	std::vector<glm::vec3> pcVertices;
	std::vector<glm::vec3> pcColors;

	//GL Box Variables
	GLuint vboBox[2];
	std::vector<glm::vec3> boxVertices;
	std::vector<unsigned int> boxIndices;

private:
	std::vector<unsigned char> bitmasksVector;
	std::vector<unsigned long int> numPointsVector;

	//Functions
public:
	PC_Viewer();
	PC_Viewer(std::string _pathFolder);

	~PC_Viewer();

	void drawBox();

private:
	void getLeafNames(std::string currentLeafName);
	void readCloudJs(std::string filename);
	void readHrcFile(std::string filename);
	void printOctree(OctreeBoxViewer level, std::string levelString);
	void printOctree(OctreeBoxViewer level, std::vector<int> levelInt);
	glm::mat4 getModelMatrixBB(glm::vec3 parentBoxMin, glm::vec3 parentBoxMax, int nextLevel, glm::vec3 & currentBoxMin, glm::vec3 & currentBoxMax);
	void loadAllPoints(std::string rootFolder, std::vector<int> currentLevel);
	void loadAllPoints(std::string rootFolder);
	void readBinaryFile(std::string filename);

	void scaleBoundingBox();

	void uploadGlBox();

	



};

