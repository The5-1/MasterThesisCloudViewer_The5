#include "PC_Viewer.h"
#include <queue>          // std::queue


PC_Viewer::PC_Viewer()
{
}

PC_Viewer::PC_Viewer(std::string _pathFolder)
{
	std::cout << "count of root->childs: " << this->root.childs.size() << std::endl;

	this->pathFolder = _pathFolder; //"D:/Dev/Assets/Pointcloud/ATL_RGB_vehicle_scan-20171228T203225Z-001/ATL_RGB_vehicle_scan/Potree"
	
	this->scaleBoundingBox();
	
	std::cout << "Start Octree construction" << std::endl;
	this->readHrcFile(this->pathFolder + "/data/r/r.hrc");
	
	std::cout << "Start printing" << std::endl;
	this->printOctree(this->root, "r");
}


PC_Viewer::~PC_Viewer()
{
}

void PC_Viewer::getLeafNames(std::string currentLeafName)
{
}

void PC_Viewer::readCloudJs(std::string filename) {

	std::string cloudJsPath = filename + "/cloud.js";

	//FILE * file;
	//fopen_s(&file, cloudJsPath.c_str(), "r");
	//
	//if (file == NULL) {
	//	std::cerr << "Model file not found: " << cloudJsPath << std::endl;
	//	exit(0);
	//}


	//while (1)
	//{
	//	char lineHeader[128];
	//	int res = fscanf(file, "%s", lineHeader);
	//	if (res == EOF) {
	//		break;
	//	}
	//
	//	if (strcmp(lineHeader, "    points:") == 0)
	//	{
	//		fscanf(file, "%f %f %f %f %f %f %f %f %f %f\n", &vertex.x, &vertex.y, &vertex.z, &normal.x, &normal.y, &normal.z, &radius, &color.x, &color.y, &color.z);
	//	}
	//
	//
	//}


	//char lineHeader[128];

	/*
	while (fgets(lineHeader, 128, file))
	{
		printf("%s", lineHeader);
		if (strcmp(lineHeader, "\"points:\"") == 0) {
			std::cout << "!!!!!!!!!!!!!!!!" << std::endl;

		}
	}
	*/

	std::fstream fs;
	std::cout << cloudJsPath << std::endl;
	fs.open(cloudJsPath);
	std::string s;
	while (std::getline(fs,s))
	{
		std::cout << s << std::endl;
		std::size_t found = s.find("points");
		if (found != std::string::npos) {
		}
	}
	fs.close();

	//fclose(file);
}

void PC_Viewer::readHrcFile(std::string filename) {
	std::ifstream file_to_open(filename, std::ios::binary);
	unsigned long int numPoints;
	unsigned char bitMaskChar; //Unsigned char has a size of 1 byte ( = 8 bits)
	std::bitset<8> bitMask;
	int numLeafs = 0;

	//Read root
	file_to_open.read((char*)&bitMaskChar, sizeof(unsigned char));
	//this->bitmasksVector.push_back(bitMaskChar);
	this->root.bitMaskChar = bitMaskChar;

	file_to_open.read((char*)&numPoints, sizeof(unsigned long int));
	//this->numPointsVector.push_back(numPoints);
	this->root.numPoints = numPoints;

	std::queue<OctreeBoxViewer*> nextLeafs;
	//Transfrom unsigned char to bits: https://stackoverflow.com/questions/37487528/how-to-get-the-value-of-every-bit-of-an-unsigned-char
	std::cout << "size of OctreeBox Class: " << sizeof(OctreeBoxViewer) << std::endl;
	std::cout << "size of root->childs: " << sizeof(this->root.childs) << std::endl;
	std::cout << "count of root->childs: " << this->root.childs.size() << std::endl;
	
	for (int i = 0; i < 8; i++) {
		bitMask[i] = (bitMaskChar & (1 << i)) != 0;

		if (bitMask[i] == 1) {
			std::cout << "size " << this->root.childs.size() << std::endl;
			std::cout << "Roots childs #" << i << std::endl;
			this->root.childs.push_back(OctreeBoxViewer());
			nextLeafs.push(&root.childs[numLeafs]);
			numLeafs++;
		}
	}

	
	std::cout << "Rest" << std::endl;
	while (!nextLeafs.empty()) {
		file_to_open.read((char*)&bitMaskChar, sizeof(unsigned char));
		file_to_open.read((char*)&numPoints, sizeof(unsigned long int));

		(*nextLeafs.front()).bitMaskChar = bitMaskChar;
		(*nextLeafs.front()).numPoints = numPoints;

		numLeafs = 0;
		for (int i = 0; i < 8; i++) {
			bitMask[i] = (bitMaskChar & (1 << i)) != 0;
			if (bitMask[i] == 1) {
				(*nextLeafs.front()).childs.push_back(OctreeBoxViewer());
				nextLeafs.push(&(*nextLeafs.front()).childs[numLeafs]);
				numLeafs++;
			}
		}

		nextLeafs.pop();
	}
}

void PC_Viewer::printOctree(OctreeBoxViewer level, std::string levelString) {
	std::bitset<8> bitMask;
	int numLeafs = 0;
	
	for (int i = 0; i != 8; i++) {
		bitMask[i] = (level.bitMaskChar & (1 << i)) != 0;
	
		if (bitMask[i] == 1) {
			std::cout << levelString + std::to_string(i) << std::endl;
			printOctree( level.childs[numLeafs], levelString + std::to_string(i));
			numLeafs++;
		}
	}
}

glm::mat4 PC_Viewer::getModelMatrixBB(glm::vec3 parentBoxMin, glm::vec3 parentBoxMax, int nextLevel, glm::vec3 &currentBoxMin, glm::vec3 &currentBoxMax) {
	//switch (nextLevel) {
	//case 0:
	//	currentBoxMin = parentBoxMin;
	//	currentBoxMax =
	//}


	//glm::mat4 modelMatrix(1.0f);

	//if (rootLevel.empty()) {
	//	modelMatrix = glm::scale(modelMatrix, glm::vec3(float(boundingBoxMaxX), float(boundingBoxMaxY), float(boundingBoxMaxZ)));
	//	return modelMatrix;
	//}

	//for (int i = 0; i < rootLevel.size(); i++) {
	//
	//}

	return glm::mat4(1.0f);
}

void PC_Viewer::loadAllPoints(std::string rootFolder, std::vector<int> currentLevel) {

	
	if (currentLevel.empty()) {
		std::string cloudJsPath = rootFolder + "/r.bin";
		this->readBinaryFile(cloudJsPath);
	}
	
}


void PC_Viewer::readBinaryFile(std::string filename) {
	std::ifstream file_to_open(filename, std::ios::binary);

	float fileScale = 0.01f;

	if (file_to_open.is_open()) {
		while (!file_to_open.eof()) {

			//glm::vec3 BBmin(2240519.12, 1363315.304, 1028.255);
			glm::vec3 BBmin(0.0, 0.0, 0.0);

			unsigned int X;
			unsigned int Y;
			unsigned int Z;
			unsigned char R;
			unsigned char G;
			unsigned char B;
			unsigned char A; //We dont use this

			file_to_open.read((char*)&X, sizeof(unsigned int));
			file_to_open.read((char*)&Y, sizeof(unsigned int));
			file_to_open.read((char*)&Z, sizeof(unsigned int));
			file_to_open.read((char*)&R, sizeof(unsigned char));
			file_to_open.read((char*)&G, sizeof(unsigned char));
			file_to_open.read((char*)&B, sizeof(unsigned char));
			file_to_open.read((char*)&A, sizeof(unsigned char));

			float Xf = ((float)X)*fileScale + BBmin.x;
			float Yf = ((float)Y)*fileScale + BBmin.y;
			float Zf = ((float)Z)*fileScale + BBmin.z;

			this->pcVertices.push_back(glm::vec3(Xf, Yf, Zf));
			this->pcColors.push_back(glm::vec3(float(R) / 255.0f, float(G) / 255.0f, float(B) / 255.0f));
		}
	}
}

void PC_Viewer::scaleBoundingBox() {
	this->boundingBoxMinX = 0.0;
	this->boundingBoxMinY = 0.0;
	this->boundingBoxMinZ = 0.0;

	this->boundingBoxMaxX -= this->boundingBoxMinX;
	this->boundingBoxMaxY -= this->boundingBoxMinY;
	this->boundingBoxMaxZ -= this->boundingBoxMinZ;
}


//Helper Functions
void PC_Viewer::uploadGlBox()
{
	boxVertices = { glm::vec3(1.0, 0.0, 1.0),
		glm::vec3(0.0, 0.0, 1.0),
		glm::vec3(0.0, 0.0, 0.0),
		glm::vec3(1.0, 0.0, 0.0),
		glm::vec3(1.0, 1.0, 0.0),
		glm::vec3(1.0, 1.0, 1.0),
		glm::vec3(0.0, 1.0, 1.0),
		glm::vec3(0.0, 1.0, 0.0)
	};

	boxIndices = { 0,1,2,3,
		0,3,4,5,
		0,5,6,1,
		1,6,7,2,
		7,4,3,2,
		4,7,6,5 };


	glGenBuffers(2, this->vboBox);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboBox[0]);
	glBufferData(GL_ARRAY_BUFFER, this->boxVertices.size() * sizeof(float) * 3, this->boxVertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboBox[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->boxIndices.size() * sizeof(unsigned int), this->boxIndices.data(), GL_STATIC_DRAW);
}

void PC_Viewer::drawBox()
{
	//Enable wireframe mode
	glPolygonMode(GL_FRONT, GL_LINE);
	glPolygonMode(GL_BACK, GL_LINE);

	//Draw vertices as glQuads
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboBox[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboBox[1]);
	glDrawElements(GL_QUADS, this->boxIndices.size(), GL_UNSIGNED_INT, 0);

	//Disable wireframe mode
	glPolygonMode(GL_FRONT, GL_FILL);
	glPolygonMode(GL_BACK, GL_FILL);
}