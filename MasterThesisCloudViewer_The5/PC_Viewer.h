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
#include <queue>          // std::queue

#include "helper.h"

struct OctreeBoxViewer {
public:
	unsigned long int numPoints = 0;
	unsigned char bitMaskChar = 0;
	glm::vec3 minLeafBox, maxLeafBox;

	std::vector<OctreeBoxViewer*> childs;

	bool drawn = false;

public:

	OctreeBoxViewer()
	{
	}

	//~OctreeBoxViewer() = default;
};

struct DynamicVBOloader {
	OctreeBoxViewer* octree;
	std::string name;
	float lod;
};

class PC_Viewer
{
	//Variables
public:
	float customScale = 0.15f;

	OctreeBoxViewer root;

	std::string pathFolder; //"D:/Dev/Assets/Pointcloud/ATL_RGB_vehicle_scan-20171228T203225Z-001/ATL_RGB_vehicle_scan/Potree"

	int points = 723386;

	double boundingBoxMinX = 2240519.12;
	double boundingBoxMinY = 1363315.304;
	double boundingBoxMinZ = 1028.255;

	double boundingBoxMaxX = 2240951.213;
	double boundingBoxMaxY = 1363747.3969999999;
	double boundingBoxMaxZ = 1460.3479999998772;

	double spacing = 3.742035150527954;
	float scale = 0.01f;
	int hierarchyStepSize = 5;

	//GL Point Cloud
	GLuint vboPC[2];
	std::vector<glm::vec3> pcVertices;
	std::vector<glm::vec3> pcColors;

	/*************
	Dynamic VBO loader
	*************/
	std::vector<GLuint> dynamicOctreeVBOs;
	std::vector<std::string> dynamicOctreeNames;
	std::vector<DynamicVBOloader> dynamicLoaders;
	unsigned int dynamicMaxVertices = 2000000;


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

	void uploadPointCloud();

	void drawPointCloud();

	void scaleVertices(float scalar);

	void octreeModelMatrix(OctreeBoxViewer level, std::vector<glm::mat4>& modelMatrixBox);

	void dynamicSetMaximumVertices(unsigned int _max);

	void dynamicSetOctreeVBOs(unsigned int _max);

	bool onCorrectPlaneSide(glm::vec3 & corner, glm::vec3 & normal, glm::vec3 & point);

	int boxFrstrumCull(OctreeBoxViewer & leaf, glm::vec3 & normal, glm::vec3 & point);

	bool cullWithViewFrustrum(OctreeBoxViewer & leaf, viewFrustrum & vF);

	void dynamicVBOload(OctreeBoxViewer level, std::string levelString, float fov, float screenHeight, glm::vec3 camPos, float minimumLOD);

public:
	void getLeafNames(std::string currentLeafName);
	void readCloudJs(std::string filename);
	void readHrcFile(std::string filename);
	void printOctree(OctreeBoxViewer level, std::string levelString);
	void printOctreeWithLOD(OctreeBoxViewer level, std::string levelString, float fov, float screenHeight, glm::vec3 camPos);
	void setBoundingBoxLevels(OctreeBoxViewer level);
	
	void loadAllPointsFromLevelToLeafs(OctreeBoxViewer level, std::string levelString);

	void loadIndexedPointsFromLevelToLeafs(OctreeBoxViewer level, std::string levelString, std::vector<int> index);

	void readBinaryFile(std::string filename, glm::vec3 boundingBoxMin);

	void scaleBoundingBox();

	void octreeForLeaf(glm::vec3 upperMin, glm::vec3 upperMax, int leaf, glm::vec3 & currentMin, glm::vec3 & currentMax);

	void uploadGlBox();

	



};

