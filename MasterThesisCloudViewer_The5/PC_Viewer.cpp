#include "PC_Viewer.h"
#include <cstdint>
#include "helper.h" //For viewfrustrum

PC_Viewer::PC_Viewer()
{
}

PC_Viewer::PC_Viewer(std::string _pathFolder)
{
	//std::cout << "count of root->childs: " << this->root.childs.size() << std::endl;

	this->pathFolder = _pathFolder; //"D:/Dev/Assets/Pointcloud/ATL_RGB_vehicle_scan-20171228T203225Z-001/ATL_RGB_vehicle_scan/Potree"
	
	this->scaleBoundingBox();
	
	this->root->minLeafBox = glm::vec3(float(this->boundingBoxMinX), float(this->boundingBoxMinY), float(this->boundingBoxMinZ));
	this->root->maxLeafBox = glm::vec3(float(this->boundingBoxMaxX), float(this->boundingBoxMaxY), float(this->boundingBoxMaxZ));

	std::cout << "Start Octree construction" << std::endl;
	this->readHrcFile(this->pathFolder + "/data/r/r.hrc");
	
	std::cout << "Set Octree Bounding boxes" << std::endl;
	this->setBoundingBoxLevels(*this->root);

	std::cout << "Load all points" << std::endl;
	this->loadAllPointsFromLevelToLeafs(*this->root, "r");

	//this->scaleVertices(1.0f);
	//this->scaleVertices(0.08f);

	this->uploadPointCloud();

	this->uploadGlBox();


	//Helper
	//this->initboxViewerVector(*this->root, "r");
	//this->boxViewerTest(*this->root, "r");
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

/***********
Used to construct the octree
***********/
void PC_Viewer::readHrcFile(std::string filename) {
	std::ifstream file_to_open(filename, std::ios::binary);
	unsigned long int numPoints;
	unsigned char bitMaskChar; //Unsigned char has a size of 1 byte ( = 8 bits)
	std::bitset<8> bitMask;
	int numLeafs = 0;

	//Read root
	file_to_open.read((char*)&bitMaskChar, sizeof(unsigned char));
	this->root->bitMaskChar = bitMaskChar;

	file_to_open.read((char*)&numPoints, sizeof(unsigned long int));
	this->root->numPoints = numPoints;

	std::queue<OctreeBoxViewer*> nextLeafs;

	for (int i = 0; i < 8; i++) {
		bitMask[i] = (bitMaskChar & (1 << i)) != 0;

		if (bitMask[i] == 1) {
			//std::cout << "size " << this->root.childs.size() << std::endl;
			//std::cout << "Roots childs #" << i << std::endl;

			OctreeBoxViewer *nextBox = new OctreeBoxViewer();

			this->root->childs.push_back(nextBox);
			nextLeafs.push(nextBox);
			numLeafs++;
		}
	}

	while (!nextLeafs.empty()) {
		//std::cout << "Queue with size: " << nextLeafs.size() << std::endl;

		OctreeBoxViewer* front = nextLeafs.front();

		file_to_open.read((char*)&bitMaskChar, sizeof(unsigned char));
		file_to_open.read((char*)&numPoints, sizeof(unsigned long int));

		(*front).bitMaskChar = bitMaskChar;
		(*front).numPoints = numPoints;

		//std::cout << (*front).bitMaskChar << " " << bitMaskChar << std::endl;
		//std::cout << (*front).numPoints << " " << numPoints << std::endl;

		numLeafs = 0;
		for (int i = 0; i < 8; i++) {
			bitMask[i] = (bitMaskChar & (1 << i)) != 0;
			if (bitMask[i] == 1) {
				OctreeBoxViewer* childBox = new OctreeBoxViewer();
				front->childs.push_back(childBox);
				nextLeafs.push(childBox);
				numLeafs++;
			}
		}

		//delete nextLeafs.front();
		nextLeafs.pop();
	}
}

//void PC_Viewer::readHrcFileWithBoundingBox(std::string filename) {
//	std::ifstream file_to_open(filename, std::ios::binary);
//	unsigned long int numPoints;
//	unsigned char bitMaskChar; //Unsigned char has a size of 1 byte ( = 8 bits)
//	std::bitset<8> bitMask;
//	int numLeafs = 0;
//
//	//Read root
//	file_to_open.read((char*)&bitMaskChar, sizeof(unsigned char));
//	this->root.bitMaskChar = bitMaskChar;
//
//	file_to_open.read((char*)&numPoints, sizeof(unsigned long int));
//	this->root.numPoints = numPoints;
//
//	this->root.minLeafBox = glm::vec3(float(this->boundingBoxMinX), float(this->boundingBoxMinY), float(this->boundingBoxMinZ));
//	this->root.maxLeafBox = glm::vec3(float(this->boundingBoxMaxX), float(this->boundingBoxMaxY), float(this->boundingBoxMaxZ));
//
//	std::queue<OctreeBoxViewer*> nextLeafs;
//
//	for (int i = 0; i < 8; i++) {
//		bitMask[i] = (bitMaskChar & (1 << i)) != 0;
//
//		if (bitMask[i] == 1) {
//			OctreeBoxViewer *nextBox = new OctreeBoxViewer();
//			this->octreeForLeaf(this->root.minLeafBox, this->root.maxLeafBox, i, nextBox->minLeafBox, nextBox->maxLeafBox);
//			this->root.childs.push_back(nextBox);
//			nextLeafs.push(nextBox);
//			numLeafs++;
//		}
//	}
//
//	while (!nextLeafs.empty()) {
//		//std::cout << "Queue with size: " << nextLeafs.size() << std::endl;
//
//		OctreeBoxViewer* front = nextLeafs.front();
//
//		file_to_open.read((char*)&bitMaskChar, sizeof(unsigned char));
//		file_to_open.read((char*)&numPoints, sizeof(unsigned long int));
//
//		(*front).bitMaskChar = bitMaskChar;
//		(*front).numPoints = numPoints;
//
//		//std::cout << (*front).bitMaskChar << " " << bitMaskChar << std::endl;
//		//std::cout << (*front).numPoints << " " << numPoints << std::endl;
//
//		numLeafs = 0;
//		for (int i = 0; i < 8; i++) {
//			bitMask[i] = (bitMaskChar & (1 << i)) != 0;
//			if (bitMask[i] == 1) {
//				OctreeBoxViewer* childBox = new OctreeBoxViewer();
//				front->childs.push_back(childBox);
//				nextLeafs.push(childBox);
//				numLeafs++;
//			}
//		}
//
//		//delete nextLeafs.front();
//		nextLeafs.pop();
//	}
//}

void PC_Viewer::printOctree(OctreeBoxViewer level, std::string levelString) {
	std::bitset<8> bitMask;
	int numLeafs = 0;

	//std::cout << levelString << " with Box: " << "(" << level.minLeafBox.x << "," << level.minLeafBox.y << "," << level.minLeafBox.z << ") to " <<
	//	"(" << level.maxLeafBox.x << "," << level.maxLeafBox.y << "," << level.maxLeafBox.z << ")" << std::endl;

	std::cout << levelString << ": (name: " << level.name << ") " << " (drawn: " << level.drawn << ") " << " (vboId: " << level.vboID << ") " << " (lod: " << level.lod << ") " << std::endl;

	for (int i = 0; i < 8; i++) {
		bitMask[i] = (level.bitMaskChar & (1 << i)) != 0;
		if (bitMask[i] == 1) {
			this->printOctree(*level.childs[numLeafs], levelString + std::to_string(i) );
			numLeafs++;
		}
	}
}

void PC_Viewer::printOctreeWithLOD(OctreeBoxViewer level, std::string levelString, float fov, float screenHeight, glm::vec3 camPos) {
	std::bitset<8> bitMask;
	int numLeafs = 0;

	//LOD:	slope = tan( fov / 2)
	//		projectedSize = (screenHeight / 2) * (radius / (slope * distance) );

	float angle = 2.0f * glm::pi<float>() * (fov / 360.0f);

	float slope = glm::tan(angle / 2.0f);

	glm::vec3 midpoint = level.minLeafBox + (level.maxLeafBox - level.minLeafBox);
	float radius = 0.5f * glm::length(level.maxLeafBox - level.minLeafBox);
	float distance = glm::length(midpoint - camPos);

	float projectedSize = (screenHeight / 2) * (radius / (slope * distance));;

	std::cout << levelString << " with Box: " << "(" << level.minLeafBox.x << "," << level.minLeafBox.y << "," << level.minLeafBox.z << ") to " <<
		"(" << level.maxLeafBox.x << "," << level.maxLeafBox.y << "," << level.maxLeafBox.z << "), LOD: " << projectedSize << std::endl;



	for (int i = 0; i < 8; i++) {
		bitMask[i] = (level.bitMaskChar & (1 << i)) != 0;
		if (bitMask[i] == 1) {
			this->printOctreeWithLOD(*level.childs[numLeafs], levelString + std::to_string(i), fov, screenHeight, camPos);
			numLeafs++;
		}
	}
}

void PC_Viewer::setBoundingBoxLevels(OctreeBoxViewer level) {
	std::bitset<8> bitMask;
	int numLeafs = 0;

	for (int i = 0; i < 8; i++) {
		bitMask[i] = (level.bitMaskChar & (1 << i)) != 0;

		if (bitMask[i] == 1) {
			this->octreeForLeaf(level.minLeafBox, level.maxLeafBox, i, level.childs[numLeafs]->minLeafBox, level.childs[numLeafs]->maxLeafBox);
			this->setBoundingBoxLevels(*level.childs[numLeafs]);
			numLeafs++;
		}
	}
}

void PC_Viewer::loadAllPointsFromLevelToLeafs(OctreeBoxViewer level, std::string levelString) {

	std::bitset<8> bitMask;
	int numLeafs = 0;

	//this->readBinaryFile(this->pathFolder + "/data/r/" + levelString + ".bin", glm::vec3(0.0f));

	//this->readBinaryFile(this->pathFolder + "/data/r/" + levelString + ".bin", glm::vec3(10.0f));

	//std::cout << this->pathFolder + "/data/r/" + levelString + ".bin:   " << level.minLeafBox.x << " , " << level.minLeafBox.y << " , " << level.minLeafBox.z << std::endl;
	this->readBinaryFile(this->pathFolder + "/data/r/" + levelString + ".bin", level.minLeafBox);

	for (int i = 0; i < 8; i++) {
		bitMask[i] = (level.bitMaskChar & (1 << i)) != 0;
		if (bitMask[i] == 1) {
			this->loadAllPointsFromLevelToLeafs(*level.childs[numLeafs], levelString + std::to_string(i));
			numLeafs++;
		}
	}

}

void PC_Viewer::loadIndexedPointsFromLevelToLeafs(OctreeBoxViewer level, std::string levelString, std::vector<int> index) {

	std::bitset<8> bitMask;
	int numLeafs = 0;

	std::string indexedLevel = "r";

	for (int i = 0; i < index.size(); i++) {
		if (index[i] >= 0) {
			indexedLevel = indexedLevel + std::to_string(index[i]);
		}
	}

	if (levelString.compare(indexedLevel) == 0) {
		std::cout << "loadIndexedPointsFromLevelToLeafs: Drawing " << levelString << std::endl;
		this->readBinaryFile(this->pathFolder + "/data/r/" + levelString + ".bin", level.minLeafBox);
	}

	for (int i = 0; i < 8; i++) {
		bitMask[i] = (level.bitMaskChar & (1 << i)) != 0;
		if (bitMask[i] == 1) {
			this->loadIndexedPointsFromLevelToLeafs(*level.childs[numLeafs], levelString + std::to_string(i), index);
			numLeafs++;
		}
	}

}

void PC_Viewer::readBinaryFile(std::string filename, glm::vec3 boundingBoxMin) {

	std::ifstream file_to_open(filename, std::ios::binary);
	float fileScale = 0.01f;

	if (file_to_open.is_open()) {
		while (!file_to_open.eof()) {
		
			//Read from file
			std::uint32_t X;
			std::uint32_t Y;
			std::uint32_t Z;

			std::uint8_t R;
			std::uint8_t G;
			std::uint8_t B;
			std::uint8_t A;

			file_to_open.read( (char*)&X, sizeof(std::uint32_t) );
			file_to_open.read( (char*)&Y, sizeof(std::uint32_t) );
			file_to_open.read( (char*)&Z, sizeof(std::uint32_t) );
			file_to_open.read( (char*)&R, sizeof(std::uint8_t) );
			file_to_open.read( (char*)&G, sizeof(std::uint8_t) );
			file_to_open.read( (char*)&B, sizeof(std::uint8_t) );
			file_to_open.read( (char*)&A, sizeof(std::uint8_t) );

			//float Xf = customScale * ( (X) * fileScale + boundingBoxMin.x );
			//float Yf = customScale * ( (Y) * fileScale + boundingBoxMin.y );
			//float Zf = customScale * ( (Z) * fileScale + boundingBoxMin.z );
			
			float Xf = this->customScale * ((float)X)* fileScale + boundingBoxMin.x;
			float Yf = this->customScale * ((float)Y)* fileScale + boundingBoxMin.y;
			float Zf = this->customScale * ((float)Z)* fileScale + boundingBoxMin.z;


			/*
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
			
			loat Xf = customScale * (((float)X) * fileScale + boundingBoxMin.x);
			loat Yf = customScale * (((float)Y) * fileScale + boundingBoxMin.y);
			loat Zf = customScale * (((float)Z) * fileScale + boundingBoxMin.z);
			*/

			/*
			double customScale = 0.05;

			std::uint32_t X;
			std::uint32_t Y;
			std::uint32_t Z;

			std::uint8_t R;
			std::uint8_t G;
			std::uint8_t B;
			std::uint8_t A;

			file_to_open.read((char*)&X, sizeof(std::uint32_t));
			file_to_open.read((char*)&Y, sizeof(std::uint32_t));
			file_to_open.read((char*)&Z, sizeof(std::uint32_t));
			file_to_open.read((char*)&R, sizeof(std::uint8_t));
			file_to_open.read((char*)&G, sizeof(std::uint8_t));
			file_to_open.read((char*)&B, sizeof(std::uint8_t));
			file_to_open.read((char*)&A, sizeof(std::uint8_t));

			double Xd = customScale * ((X)* fileScale + boundingBoxMin.x);
			double Yd = customScale * ((Y)* fileScale + boundingBoxMin.y);
			double Zd = customScale * ((Z)* fileScale + boundingBoxMin.z);

			float Xf = (float)Xd;
			float Yf = (float)Yd;
			float Zf = (float)Zd;
			*/

			//Write results in temp vector
			this->pcVertices.push_back(glm::vec3(Xf, Yf, Zf));
			this->pcColors.push_back(glm::vec3(float(R) / 255.0f, float(G) / 255.0f, float(B) / 255.0f));
			
		}
	}
}

void PC_Viewer::scaleBoundingBox() {
	this->boundingBoxMaxX -= this->boundingBoxMinX;
	this->boundingBoxMaxY -= this->boundingBoxMinY;
	this->boundingBoxMaxZ -= this->boundingBoxMinZ;

	this->boundingBoxMinX = 0.0;
	this->boundingBoxMinY = 0.0;
	this->boundingBoxMinZ = 0.0;

	this->boundingBoxMaxX *= this->customScale;
	this->boundingBoxMaxY *= this->customScale;
	this->boundingBoxMaxZ *= this->customScale;

	std::cout << "Box min: (" << boundingBoxMinX << "," << boundingBoxMinY <<","<< boundingBoxMinZ << ")" << std::endl;
	std::cout << "Box max (" << boundingBoxMaxX << "," << boundingBoxMaxY << "," << boundingBoxMaxZ << ")" << std::endl;
}


void PC_Viewer::octreeForLeaf(glm::vec3 upperMin, glm::vec3 upperMax, int leaf, glm::vec3 &currentMin, glm::vec3 &currentMax) {

	glm::vec3 direction = (upperMax - upperMin);

	switch (leaf) {
				//Zero
		case 0: {
			currentMin = upperMin + glm::vec3(0.0f, 0.0f, 0.0f);
			currentMax = upperMin + glm::vec3(0.5f * direction.x, 0.5f * direction.y, 0.5f * direction.z);
			break;
		}
				//One
		case 2: {
			currentMin = upperMin + glm::vec3(0.0f, 0.5f * direction.y, 0.0f);
			currentMax = upperMin + glm::vec3(0.5f * direction.x, direction.y, 0.5f * direction.z);
			break;
		}
				//Two
		case 1: {
			currentMin = upperMin + glm::vec3(0.0f, 0.0f, 0.5f * direction.z);
			currentMax = upperMin + glm::vec3(0.5f * direction.x, 0.5f * direction.y, direction.z);
			break;
		}
				//Three
		case 3: {
			currentMin = upperMin + glm::vec3(0.0f, 0.5f * direction.y, 0.5f * direction.z);
			currentMax = upperMin + glm::vec3(0.5f * direction.x, direction.y, direction.z);
			break;
		}
		
		//Right
				//Four
		case 4: {
			currentMin = upperMin + glm::vec3(0.5f * direction.x, 0.0f, 0.0f);
			currentMax = upperMin + glm::vec3(direction.x, 0.5f * direction.y, 0.5f * direction.z);
			break;
		}
				//Five
		case 6: {
			currentMin = upperMin + glm::vec3(0.5f * direction.x, 0.5f * direction.y, 0.0f);
			currentMax = upperMin + glm::vec3(direction.x, direction.y, 0.5f * direction.z);
			break;
		}
				//Six
		case 5: {
			currentMin = upperMin + glm::vec3(0.5f * direction.x, 0.0f, 0.5f * direction.z);
			currentMax = upperMin + glm::vec3(direction.x, 0.5f * direction.y, direction.z);
			break;
		}
				//Seven
		case 7: {
			currentMin = upperMin + glm::vec3(0.5f * direction.x, 0.5f * direction.y, 0.5f * direction.z);
			currentMax = upperMin + glm::vec3(direction.x, direction.y, direction.z);
			break;
		}
	}
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

void PC_Viewer::uploadPointCloud() {
	glGenBuffers(2, this->vboPC);

	glBindBuffer(GL_ARRAY_BUFFER, this->vboPC[0]);
	glBufferData(GL_ARRAY_BUFFER, this->pcVertices.size() * sizeof(float) * 3, this->pcVertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->vboPC[1]);
	glBufferData(GL_ARRAY_BUFFER, this->pcColors.size() * sizeof(float) * 3, this->pcColors.data(), GL_STATIC_DRAW);
}

void PC_Viewer::drawPointCloud() {
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboPC[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboPC[1]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawArrays(GL_POINTS, 0, this->pcVertices.size());
}

void PC_Viewer::scaleVertices(float scalar) {
	for (int i = 0; i < this->pcVertices.size(); i++) {
		this->pcVertices[i] *= scalar;

		//float Xo, Yo, Zo;
		//Xo = this->pcVertices[i].x;
		//Yo = this->pcVertices[i].y;
		//Zo = this->pcVertices[i].z;

		//this->pcVertices[i].x = Zo;
		//this->pcVertices[i].y = Yo;
		//this->pcVertices[i].z = Xo;
	}
}

void PC_Viewer::octreeModelMatrix(OctreeBoxViewer level, std::vector<glm::mat4> &modelMatrixBox) {
	std::bitset<8> bitMask;
	int numLeafs = 0;

	glm::mat4 _modelMatrix = glm::mat4(1.0f);
	_modelMatrix = glm::translate(_modelMatrix,level.minLeafBox);
	glm::vec3 scaleVec = glm::abs(level.maxLeafBox - level.minLeafBox);
	_modelMatrix = glm::scale(_modelMatrix, scaleVec);

	modelMatrixBox.push_back(_modelMatrix);

	for (int i = 0; i < 8; i++) {
		bitMask[i] = (level.bitMaskChar & (1 << i)) != 0;
		if (bitMask[i] == 1) {
			this->octreeModelMatrix(*level.childs[numLeafs], modelMatrixBox);
			numLeafs++;
		}
	}

}



/***********
Dynamic Octree Load
***********/
void PC_Viewer::dynamicSetMaximumVertices(int _max) {
	this->dynamicMaxVertices = _max;
}

void PC_Viewer::dynamicSetOctreeVBOs(int _max) {
	glDeleteBuffers(this->dynamicOctreeVBOs.size(), this->dynamicOctreeVBOs.data());
	this->dynamicOctreeVBOs.resize(2 * _max); //Buffers for positions and colors
	glGenBuffers(this->dynamicOctreeVBOs.size(), this->dynamicOctreeVBOs.data());

	this->pcVertices.clear();
	this->pcColors.clear();

	//for (int i = 0; i < _max; i++) {
	//	glBindBuffer(GL_ARRAY_BUFFER, i);
	//	glBufferData(GL_ARRAY_BUFFER, this->pcVertices.size() * sizeof(float) * 3, this->pcVertices.data(), GL_STATIC_DRAW);
	//
	//	glBindBuffer(GL_ARRAY_BUFFER, i + _max);
	//	glBufferData(GL_ARRAY_BUFFER, this->pcColors.size() * sizeof(float) * 3, this->pcColors.data(), GL_STATIC_DRAW);
	//}

	for (int i = 0; i < _max; i++) {
		this->dynamicQueue.push(i);
	}
}

void PC_Viewer::dynamicLoadRoot() {
	glGenBuffers(2, this->vboRootPC);

	std::string treePath = this->pathFolder + "/data/r/" + "r" + ".bin";
	glm::vec3 treeMinLeaf = this->root->minLeafBox;
	this->readBinaryFile(treePath, treeMinLeaf);

	this->vboRootSizePC = this->pcVertices.size();

	glBindBuffer(GL_ARRAY_BUFFER, this->vboRootPC[0]);
	glBufferData(GL_ARRAY_BUFFER, this->pcVertices.size() * sizeof(float) * 3, this->pcVertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->vboRootPC[1]);
	glBufferData(GL_ARRAY_BUFFER, this->pcColors.size() * sizeof(float) * 3, this->pcColors.data(), GL_STATIC_DRAW);

	this->pcColors.clear();
	this->pcVertices.clear();
}

bool PC_Viewer::onCorrectPlaneSide(glm::vec3& corner, glm::vec3& normal, glm::vec3& point) {
	if (glm::dot(normal, (corner - point)) > 0) {
		return true;
	}

	return false;
}


int PC_Viewer::boxFrstrumCull(OctreeBoxViewer& leaf, glm::vec3& normal, glm::vec3& point) {
	//Return 0 if box inside frustrum
	//Return 1 if box outside frustrum
	//Return 2 if box cuts Frustrum

	int counter = 0;

	if (!onCorrectPlaneSide(leaf.minLeafBox, normal, point)) {
		counter++;
	}

	if (!onCorrectPlaneSide(leaf.maxLeafBox, normal, point)) {
		counter++;
	}

	if (!onCorrectPlaneSide(glm::vec3(leaf.minLeafBox.x, leaf.minLeafBox.y, leaf.maxLeafBox.z), normal, point)) {
		counter++;
	}

	if (!onCorrectPlaneSide(glm::vec3(leaf.minLeafBox.x, leaf.maxLeafBox.y, leaf.minLeafBox.z), normal, point)) {
		counter++;
	}

	if (!onCorrectPlaneSide(glm::vec3(leaf.maxLeafBox.x, leaf.minLeafBox.y, leaf.minLeafBox.z), normal, point)) {
		counter++;
	}

	if (!onCorrectPlaneSide(glm::vec3(leaf.minLeafBox.x, leaf.maxLeafBox.y, leaf.maxLeafBox.z), normal, point)) {
		counter++;
	}

	if (!onCorrectPlaneSide(glm::vec3(leaf.maxLeafBox.x, leaf.minLeafBox.y, leaf.maxLeafBox.z), normal, point)) {
		counter++;
	}

	if (!onCorrectPlaneSide(glm::vec3(leaf.maxLeafBox.x, leaf.maxLeafBox.y, leaf.minLeafBox.z), normal, point)) {
		counter++;
	}

	if (counter == 0) {
		return 0;
	}

	if (counter == 8) {
		return 1;
	}

	return 2;
}

bool PC_Viewer::cullWithViewFrustrum(OctreeBoxViewer& leaf, viewFrustrum& vF)
{
	int partiallyInside = 0;
	int inOutTest = -1;

	//1
	inOutTest = boxFrstrumCull(leaf, vF.farNormal, vF.farPoint);
	if (inOutTest == 1) {
		//Box is compeltely outside of a plane, we can stop here
		return false;
	}
	else if (inOutTest == 2) {
		//Box is partly inside the frustrum (if its not the leave yet keep splitting, else just take the entire box)
		if (leaf.childs.size() == 0) {
			//this->addBoxToDraw(leaf, glm::vec3(1.0f, 0.0f, 0.0f));
			partiallyInside++;
		}
		else {
			for (int i = 0; i < leaf.childs.size(); i++) {
				cullWithViewFrustrum(*leaf.childs[i], vF);
			}
			return false;
		}
	}

	//2
	inOutTest = boxFrstrumCull(leaf, vF.nearNormal, vF.nearPoint);
	if (inOutTest == 1) {
		//Box is compeltely outside of a plane, we can stop here
		return false;
	}
	else if (inOutTest == 2) {
		//Box is partly inside the frustrum (if its not the leave yet keep splitting, else just take the entire box)
		if (leaf.childs.size() == 0) {
			//this->addBoxToDraw(leaf, glm::vec3(1.0f, 0.0f, 0.0f));
			partiallyInside++;
		}
		else {
			for (int i = 0; i < leaf.childs.size(); i++) {
				cullWithViewFrustrum(*leaf.childs[i], vF);
			}
			return false;
		}
	}

	//3
	inOutTest = boxFrstrumCull(leaf, vF.leftNormal, vF.leftPoint);
	if (inOutTest == 1) {
		//Box is compeltely outside of a plane, we can stop here
		return false;
	}
	else if (inOutTest == 2) {
		//Box is partly inside the frustrum (if its not the leave yet keep splitting, else just take the entire box)
		if (leaf.childs.size() == 0) {
			//this->addBoxToDraw(leaf, glm::vec3(1.0f, 0.0f, 0.0f));
			partiallyInside++;
		}
		else {
			for (int i = 0; i < leaf.childs.size(); i++) {
				cullWithViewFrustrum(*leaf.childs[i], vF);
			}
			return false;
		}
	}

	//4
	inOutTest = boxFrstrumCull(leaf, vF.rightNormal, vF.rightPoint);
	if (inOutTest == 1) {
		//Box is compeltely outside of a plane, we can stop here
		return false;
	}
	else if (inOutTest == 2) {
		//Box is partly inside the frustrum (if its not the leave yet keep splitting, else just take the entire box)
		if (leaf.childs.size() == 0) {
			//this->addBoxToDraw(leaf, glm::vec3(1.0f, 0.0f, 0.0f));
			partiallyInside++;
		}
		else {
			for (int i = 0; i < leaf.childs.size(); i++) {
				cullWithViewFrustrum(*leaf.childs[i], vF);
			}
			return false;
		}
	}


	//5
	inOutTest = boxFrstrumCull(leaf, vF.upNormal, vF.upPoint);
	if (inOutTest == 1) {
		//Box is compeltely outside of a plane, we can stop here
		return false;
	}
	else if (inOutTest == 2) {
		//Box is partly inside the frustrum (if its not the leave yet keep splitting, else just take the entire box)
		if (leaf.childs.size() == 0) {
			//this->addBoxToDraw(leaf, glm::vec3(1.0f, 0.0f, 0.0f));
			partiallyInside++;
		}
		else {
			for (int i = 0; i < leaf.childs.size(); i++) {
				cullWithViewFrustrum(*leaf.childs[i], vF);
			}
			return false;
		}
	}

	//6
	inOutTest = boxFrstrumCull(leaf, vF.downNormal, vF.downPoint);
	if (inOutTest == 1) {
		//Box is compeltely outside of a plane, we can stop here
		return false;
	}
	else if (inOutTest == 2) {
		//Box is partly inside the frustrum (if its not the leave yet keep splitting, else just take the entire box)
		if (leaf.childs.size() == 0) {
			//this->addBoxToDraw(leaf, glm::vec3(1.0f, 0.0f, 0.0f));
			partiallyInside++;
		}
		else {
			for (int i = 0; i < leaf.childs.size(); i++) {
				cullWithViewFrustrum(*leaf.childs[i], vF);
			}
			return false;
		}
	}

	//Box inside Frsutrum 
	//ToDo: Check Lod and insert into draw array if necessary
	return true;
}

void PC_Viewer::dynamicStartLoad(OctreeBoxViewer level, std::string levelString, float fov, float screenHeight, glm::vec3 camPos, viewFrustrum& vF, float minimumLOD) {
	//std::cout << "dynamicStartLoad start" << std::endl;

	this->dynamicLoaders.clear();

	//std::cout << "dynamicVBOload start" << std::endl;
	this->dynamicVBOload(level, levelString, fov, screenHeight, camPos, vF, minimumLOD);

	//for (int i = 0; i < this->dynamicLoaders.size(); i++) {
	//	std::cout << "--- pointer after dynamicVBOload " << this->dynamicLoaders[i].octree->vboID << std::endl;
	//}

	//std::cout << "dynamicVBOload end" << std::endl;
	/*
	std::cout << "Before sort" << std::endl;
	for (int i = 0; i < this->dynamicLoaders.size(); i++) {
		std::cout << this->dynamicLoaders[i].lod << std::endl;
	}
	*/

	std::sort(this->dynamicLoaders.begin(), this->dynamicLoaders.end());

	/*
	std::cout << "After sort" << std::endl;
	for (int i = 0; i < this->dynamicLoaders.size(); i++) {
		std::cout << this->dynamicLoaders[i].lod << std::endl;
	}
	std::cout << "-----------------" << std::endl;
	*/

	//std::cout << "dynamicLoaders.size " << this->dynamicLoaders.size() << std::endl;
	//std::cout << "this->dynamicOctreeVBOs.size() " << this->dynamicOctreeVBOs.size() << std::endl;

	int max = this->dynamicOctreeVBOs.size() / 2;
	int offsetVBO = max;

	//Deactivate all loaders which we dont want to draw anymore
	if (this->dynamicLoaders.size() > max) {
		for (int i = max; i < this->dynamicLoaders.size(); i++) {
			if (this->dynamicLoaders[i].octree->drawn) {
				//std::cout << "!!! Draw was true" << std::endl;
				this->dynamicLoaders[i].octree->drawn = false;
				this->dynamicQueue.push(this->dynamicLoaders[i].octree->vboID);
				this->dynamicLoaders[i].octree->vboID = -1;
			}
		}
	}

	max = std::min<int>(max, this->dynamicLoaders.size());

	//std::cout << "dynamicLoaders.size: " << this->dynamicLoaders.size() << std::endl;
	
	for (int i = 0; i < max; i++) {
		if (this->dynamicQueue.size() > 0) {
			//std::cout << "i: " << i << std::endl;
			this->dynamicLoaders[i].octree->drawn = true;
			//std::cout << "this->dynamicLoaders[i].octree->drawn = true;" << std::endl;

			this->dynamicLoaders[i].octree->vboID = this->dynamicQueue.front();
			//std::cout << "this->dynamicLoaders[i].octree->vboID = this->dynamicQueue.front();" << std::endl;

			this->dynamicQueue.pop();
			//std::cout << "this->dynamicQueue.pop();" << std::endl;

			this->pcColors.clear();
			this->pcVertices.clear();

			std::string treePath = this->pathFolder + "/data/r/" + this->dynamicLoaders[i].name + ".bin";
			glm::vec3 treeMinLeaf = this->dynamicLoaders[i].octree->minLeafBox;
			this->readBinaryFile(treePath, treeMinLeaf);

			int numPoints = this->pcVertices.size();
			this->dynamicLoaders[i].numPoints = numPoints;
			this->dynamicLoaders[i].octree->numPoints = numPoints;

			//std::cout << "numPoints " << numPoints << std::endl;
			//std::cout << "dynamic numPoints " << this->dynamicLoaders[i].numPoints << std::endl;

			//std::cout << "Test this->dynamicLoaders[i].octree->vboID:" << this->dynamicLoaders[i].octree->vboID << " + offset of "<< offsetVBO << std::endl;
			glBindBuffer(GL_ARRAY_BUFFER, this->dynamicOctreeVBOs[this->dynamicLoaders[i].octree->vboID ]);
			glBufferData(GL_ARRAY_BUFFER, this->pcVertices.size() * sizeof(float) * 3, this->pcVertices.data(), GL_STATIC_DRAW);
			//std::cout << "Bound position" << std::endl;
			glBindBuffer(GL_ARRAY_BUFFER, this->dynamicOctreeVBOs[this->dynamicLoaders[i].octree->vboID + offsetVBO]);
			glBufferData(GL_ARRAY_BUFFER, this->pcColors.size() * sizeof(float) * 3, this->pcColors.data(), GL_STATIC_DRAW);
			//std::cout << "Bound color" << std::endl;
			//std::cout << "Test end" << std::endl;
		}
		else {
			break; //ToDo: Sinnvoll? !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		}
	}

	//std::cout << "dynamicStartLoad end" << std::endl;
	//std::cout << "" << std::endl;
}

bool PC_Viewer::dynamicLodBySize(OctreeBoxViewer& level, float fov, float screenHeight, glm::vec3 camPos, viewFrustrum& vF, float minimumLOD, float &projectedSize) {

	//LOD:	slope = tan( fov / 2)
	//		projectedSize = (screenHeight / 2) * (radius / (slope * distance) );

	//std::cout << "Start dynamicVBOload" << std::endl;

	float angle = 2.0f * glm::pi<float>() * (fov / 360.0f);
	float slope = glm::tan(angle / 2.0f);


	//glm::vec3 midpoint = level.childs[numLeafs]->minLeafBox + (level.childs[numLeafs]->maxLeafBox - level.childs[numLeafs]->minLeafBox);
	//float radius = 0.5f * glm::length(level.childs[numLeafs]->maxLeafBox - level.childs[numLeafs]->minLeafBox);
	glm::vec3 midpoint = level.minLeafBox + (level.maxLeafBox - level.minLeafBox);
	float radius = 0.5f * glm::length(level.maxLeafBox - level.minLeafBox);

	float distance = glm::length(midpoint - camPos);
	projectedSize = (screenHeight / 2) * (radius / (slope * distance));;

	return projectedSize > minimumLOD;
}

bool PC_Viewer::dynamicLodByDistance(OctreeBoxViewer& level, float fov, float screenHeight, glm::vec3 camPos, viewFrustrum& vF, float minimumLOD, float &projectedSize) {
	glm::vec3 midpoint = level.minLeafBox + (level.maxLeafBox - level.minLeafBox);
	float radius = 0.5f * glm::length(level.maxLeafBox - level.minLeafBox);

	float distance = glm::length(midpoint - camPos);
	projectedSize = distance;

	return distance < minimumLOD;
}

float PC_Viewer::dynamicLodBySize(OctreeBoxViewer& level, float fov, float screenHeight, glm::vec3 camPos, viewFrustrum& vF, float minimumLOD) {
	float angle = 2.0f * glm::pi<float>() * (fov / 360.0f);
	float slope = glm::tan(angle / 2.0f);


	//glm::vec3 midpoint = level.childs[numLeafs]->minLeafBox + (level.childs[numLeafs]->maxLeafBox - level.childs[numLeafs]->minLeafBox);
	//float radius = 0.5f * glm::length(level.childs[numLeafs]->maxLeafBox - level.childs[numLeafs]->minLeafBox);
	glm::vec3 midpoint = level.minLeafBox + (level.maxLeafBox - level.minLeafBox);
	float radius = 0.5f * glm::length(level.maxLeafBox - level.minLeafBox);

	float distance = glm::length(midpoint - camPos);
	float projectedSize = (screenHeight / 2) * (radius / (slope * distance));;

	return projectedSize;
}

void PC_Viewer::dynamicVBOload(OctreeBoxViewer level, std::string levelString, float fov, float screenHeight, glm::vec3 camPos, viewFrustrum& vF, float minimumLOD) {
	std::bitset<8> bitMask;
	int numLeafs = 0;

	for (int i = 0; i < 8; i++) {
		bitMask[i] = (level.bitMaskChar & (1 << i)) != 0;
		if (bitMask[i] == 1) {
			
			float projectedSize;
			if (this->dynamicLodByDistance(*level.childs[numLeafs], fov, screenHeight, camPos, vF, minimumLOD, projectedSize)) {
				this->dynamicLoaders.push_back(DynamicVBOloader(*level.childs[numLeafs], levelString + std::to_string(i), projectedSize));
			}
			else {
				if (level.childs[numLeafs]->drawn) {
					int max = this->dynamicOctreeVBOs.size() / 2;

					level.childs[numLeafs]->drawn = false;

					this->pcColors.clear();
					this->pcVertices.clear();

					glBindBuffer(GL_ARRAY_BUFFER, this->dynamicOctreeVBOs[level.childs[numLeafs]->vboID]);
					glBufferData(GL_ARRAY_BUFFER, this->pcVertices.size() * sizeof(float) * 3, this->pcVertices.data(), GL_STATIC_DRAW);

					glBindBuffer(GL_ARRAY_BUFFER, this->dynamicOctreeVBOs[level.childs[numLeafs]->vboID + max]);
					glBufferData(GL_ARRAY_BUFFER, this->pcColors.size() * sizeof(float) * 3, this->pcColors.data(), GL_STATIC_DRAW);

					this->dynamicQueue.push(level.childs[numLeafs]->vboID);
					level.childs[numLeafs]->vboID = -1;
				}
			}

			this->dynamicVBOload(*level.childs[numLeafs], levelString + std::to_string(i), fov, screenHeight, camPos, vF, minimumLOD);
			numLeafs++;
		}
	}

	//std::cout << "End dynamicVBOload" << std::endl;
}

//void PC_Viewer::dynamicVBOload(OctreeBoxViewer level, std::string levelString, float fov, float screenHeight, glm::vec3 camPos, viewFrustrum& vF, float minimumLOD) {
//	std::bitset<8> bitMask;
//	int numLeafs = 0;
//
//	//LOD:	slope = tan( fov / 2)
//	//		projectedSize = (screenHeight / 2) * (radius / (slope * distance) );
//	float angle = 2.0f * glm::pi<float>() * (fov / 360.0f);
//	float slope = glm::tan(angle / 2.0f);
//	glm::vec3 midpoint = level.minLeafBox + (level.maxLeafBox - level.minLeafBox);
//	float radius = 0.5f * glm::length(level.maxLeafBox - level.minLeafBox);
//	float distance = glm::length(midpoint - camPos);
//	float projectedSize = (screenHeight / 2) * (radius / (slope * distance));;
//
//	if (projectedSize > minimumLOD && cullWithViewFrustrum(level, vF)) {
//		std::cout << "dynamicVBOload: " << level.vboID << std::endl;
//		this->dynamicLoaders.push_back(DynamicVBOloader(level, levelString, projectedSize));
//		std::cout << "dynamicVBOload Pointer: " << this->dynamicLoaders[this->dynamicLoaders.size() - 1].octree->vboID << std::endl;
//	}
//	else {
//		if (level.drawn) {
//			std::cout << "dynamicVBOload Level not drawn anymore: " << level.vboID << " pushed back into queue" << std::endl;
//			int max = this->dynamicOctreeVBOs.size() / 2;
//
//			level.drawn = false;
//			
//			this->pcColors.clear();
//			this->pcVertices.clear();
//
//			glBindBuffer(GL_ARRAY_BUFFER, this->dynamicOctreeVBOs[level.vboID]);
//			glBufferData(GL_ARRAY_BUFFER, this->pcVertices.size() * sizeof(float) * 3, this->pcVertices.data(), GL_STATIC_DRAW);
//
//			glBindBuffer(GL_ARRAY_BUFFER, this->dynamicOctreeVBOs[level.vboID + max]);
//			glBufferData(GL_ARRAY_BUFFER, this->pcColors.size() * sizeof(float) * 3, this->pcColors.data(), GL_STATIC_DRAW);
//
//			this->dynamicQueue.push(level.vboID);
//			level.vboID = -1;
//		}
//	}
//
//	for (int i = 0; i < 8; i++) {
//		bitMask[i] = (level.bitMaskChar & (1 << i)) != 0;
//		if (bitMask[i] == 1) {
//			this->dynamicVBOload(*level.childs[numLeafs], levelString + std::to_string(i), fov, screenHeight, camPos, vF, minimumLOD);
//			numLeafs++;
//		}
//	}
//
//	for (int i = 0; i < this->dynamicLoaders.size(); i++) {
//		std::cout << "+++ pointer after dynamicVBOload " << this->dynamicLoaders[i].name  << ", "<< this->dynamicLoaders[i].octree->vboID << std::endl;
//		//std::cout << "+++ pointer after dynamicVBOload " << this->dynamicLoaders[i].octree->debugName << ", " << this->dynamicLoaders[i].octree->vboID << std::endl;
//	}
//}

void PC_Viewer::printLoaders() {
	std::cout << "Print loaders: " << std::endl;
	for (int i = 0; i < dynamicLoaders.size(); i++) {
		std::cout << this->dynamicLoaders[i] << std::endl;
	}
}

void PC_Viewer::dynamicDraw() {
	/****************
	Draw root level
	****************/
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboRootPC[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboRootPC[1]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawArrays(GL_POINTS, 0, this->vboRootSizePC);

	/****************
	Draw
	****************/
	//std::cout << "!!! Start Drawing" << std::endl;
	
	int max = this->dynamicOctreeVBOs.size() / 2;
	
	//std::cout << "!!! Start Drawing" << std::endl;
	//std::cout << "this->dynamicOctreeVBOs " << this->dynamicOctreeVBOs.size() << std::endl;
	//std::cout << " this->dynamicLoaders " << this->dynamicLoaders.size() << std::endl;
	
	int min = std::min<int>(this->dynamicLoaders.size(), max);
	
	for (int i = 0; i < min; i++) {
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, this->dynamicOctreeVBOs[i]);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, this->dynamicOctreeVBOs[i + max]);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

			glDrawArrays(GL_POINTS, 0, this->dynamicLoaders[i].octree->numPoints);
	}
	
	//std::cout << "!!! End Drawing" << std::endl;

	/****************
	Debug
	****************/
	//for (int i = 0; i < this->dynamicLoaders.size(); i++) {
	//	std::cout << i << std::endl;
	//	std::cout << "this->dynamicLoaders[i].lod " << this->dynamicLoaders[i].lod << " , " << std::endl;
	//	std::cout << "this->dynamicLoaders[i].name " << this->dynamicLoaders[i].name << " , " << std::endl;
	//	std::cout << "this->dynamicLoaders[i].octree->numPoints " << this->dynamicLoaders[i].octree->numPoints << " , " << std::endl;
	//	std::cout << "this->dynamicLoaders[i].octree->drawn " << this->dynamicLoaders[i].octree->drawn << " , " << std::endl;
	//	std::cout << "this->dynamicLoaders[i].octree->name " << this->dynamicLoaders[i].octree->name << " , " << std::endl;
	//	std::cout << "this->dynamicLoaders[i].octree->vboID " << this->dynamicLoaders[i].octree->vboID << " , " << std::endl;
	//}
	//
	//std::cout << " -------------------------------------------- " << std::endl;
}


/***********
Cheap load (Always relaods, only usable for small data)
***********/
void PC_Viewer::fastStartLoad(OctreeBoxViewer level, std::string levelString, float fov, float screenHeight, glm::vec3 camPos, viewFrustrum& vF, float minimumLOD) {
	//Helper
	std::bitset<8> bitMask;
	int numLeafs = 0;

	//Lod
	float angle = 2.0f * glm::pi<float>() * (fov / 360.0f);
	float slope = glm::tan(angle / 2.0f);
	glm::vec3 midpoint = level.minLeafBox + (level.maxLeafBox - level.minLeafBox);
	float radius = 0.5f * glm::length(level.maxLeafBox - level.minLeafBox);
	float distance = glm::length(midpoint - camPos);
	float projectedSize = (screenHeight / 2) * (radius / (slope * distance));;

	if (projectedSize > minimumLOD && cullWithViewFrustrum(level, vF)) {
	
	}

	for (int i = 0; i < 8; i++) {
		bitMask[i] = (level.bitMaskChar & (1 << i)) != 0;
		if (bitMask[i] == 1) {
			this->fastStartLoad(*level.childs[numLeafs], levelString + std::to_string(i), fov, screenHeight, camPos, vF, minimumLOD);
			numLeafs++;
		}
	}


	this->pathFolder + "/data/r/" + levelString + ".bin";
}


void PC_Viewer::initboxViewerVector(OctreeBoxViewer level, std::string levelString) {
	//Helper
	std::bitset<8> bitMask;
	int numLeafs = 0;

	for (int i = 0; i < 8; i++) {
		bitMask[i] = (level.bitMaskChar & (1 << i)) != 0;
		if (bitMask[i] == 1) {
			this->initboxViewerVector(*level.childs[numLeafs], levelString + std::to_string(i));

			level.childs[numLeafs]->name = levelString + std::to_string(i);
			this->boxViewerVector.push_back(level.childs[numLeafs]);

			numLeafs++;
		}
	}
}

void PC_Viewer::boxViewerTest(OctreeBoxViewer level, std::string levelString) {
	std::cout << "--- Print Octree before change:" << std::endl;
	this->printOctree(*this->root, "r");

	std::cout << "--- Print Vector  before change: Size " << this->boxViewerVector.size() << std::endl;
	for (int i = 0; i < this->boxViewerVector.size(); i++) {
		//std::cout << &this->boxViewerVector << std::endl;
		std::cout << "(name: " << this->boxViewerVector[i]->name << ") " << " (drawn: " << this->boxViewerVector[i]->drawn << ") " << " (vboId: " << this->boxViewerVector[i]->vboID << ") " << " (lod: " << this->boxViewerVector[i]->lod << ") " << std::endl;
		//std::cout << "(name: " << this->boxViewerVector[i].name << ") " << " (drawn: " << this->boxViewerVector[i].drawn << ") " << " (vboId: " << this->boxViewerVector[i].vboID << ") " << " (lod: " << this->boxViewerVector[i].lod << ") " << std::endl;
	}

	for (int i = 0; i < this->boxViewerVector.size(); i++) {
		this->boxViewerVector[i]->drawn = true;
		this->boxViewerVector[i]->vboID = 500.0f;
		//this->boxViewerVector[i].drawn = true;
		//this->boxViewerVector[i].vboID = 500.0f;
	}

	std::cout << "--- Print Octree:" << std::endl;
	this->printOctree(*this->root, "r");

	std::cout << "--- Print Vector:" << std::endl;
	for (int i = 0; i < this->boxViewerVector.size(); i++) {
		std::cout << "(name: " << this->boxViewerVector[i]->name << ") " << " (drawn: " << this->boxViewerVector[i]->drawn << ") " << " (vboId: " << this->boxViewerVector[i]->vboID << ") " << " (lod: " << this->boxViewerVector[i]->lod << ") " << std::endl;
		//std::cout << "(name: " << this->boxViewerVector[i].name << ") " << " (drawn: " << this->boxViewerVector[i].drawn << ") " << " (vboId: " << this->boxViewerVector[i].vboID << ") " << " (lod: " << this->boxViewerVector[i].lod << ") " << std::endl;
	}
	
}