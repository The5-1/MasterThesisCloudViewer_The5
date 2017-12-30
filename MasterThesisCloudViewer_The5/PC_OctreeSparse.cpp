#define GLEW_STATIC //Using the static lib, so we need to enable it

#include "PC_OctreeSparse.h"

#include <glm/gtc/matrix_transform.hpp> 
#include <algorithm>

/* *********************************************************************************************************
//Functions (public)
********************************************************************************************************* */
PC_OctreeSparse::PC_OctreeSparse()
{
}

PC_OctreeSparse::PC_OctreeSparse(std::vector<glm::vec3>& _vertices, std::vector<glm::vec3>& _normals, std::vector<float>& _radius, int _maxVerticesPerQuad)
{
	this->getAABB(minBoundingBox, maxBoundingBox, _vertices);

	this->uploadGlBox();

	this->root.endVertices = _vertices.size();
	this->root.beginVertices = 0;

	this->maxVerticesPerQuad = _maxVerticesPerQuad;

	this->root.maxLeafBox = this->maxBoundingBox;
	this->root.minLeafBox = this->minBoundingBox;


	for (int i = 0; i < this->root.endVertices; i++) {
		this->vertexIndexList.push_back(i);
	}


	if (_vertices.size() < _maxVerticesPerQuad) {
		std::cout << "Model has less vertices then give max size per leaf. All informations stored in root." << std::endl;
		return;
	}

	this->vertexColorList.resize(_vertices.size());

	this->splitLeaf(this->root, _vertices);

	this->uploadPointCloud(_vertices, _normals, _radius);
}

PC_OctreeSparse::PC_OctreeSparse(std::vector<glm::vec3>& _vertices, std::vector<glm::vec3>& _normals, std::vector<glm::vec3>& _colors, std::vector<float>& _radius, int _maxVerticesPerQuad)
{
	this->getAABB(minBoundingBox, maxBoundingBox, _vertices);

	this->uploadGlBox();

	this->root.endVertices = _vertices.size();
	this->root.beginVertices = 0;

	this->maxVerticesPerQuad = _maxVerticesPerQuad;

	this->root.maxLeafBox = this->maxBoundingBox;
	this->root.minLeafBox = this->minBoundingBox;

	for (int i = 0; i < this->root.endVertices; i++) {
		this->vertexIndexList.push_back(i);
	}


	if (_vertices.size() < _maxVerticesPerQuad) {
		std::cout << "Model has less vertices then give max size per leaf. All informations stored in root." << std::endl;
		return;
	}

	this->vertexColorList.resize(_vertices.size());

	this->splitLeaf(this->root, _vertices);

	for (int i = 0; i < this->vertexColorList.size(); i++) {
		this->vertexColorList[i] = _colors[i];
	}

	this->uploadPointCloud(_vertices, _normals, _radius);
}


PC_OctreeSparse::PC_OctreeSparse(std::vector<glm::vec3>& _vertices, std::vector<glm::vec3>& _colors, int _maxVerticesPerQuad)
{
	this->getAABB(minBoundingBox, maxBoundingBox, _vertices);

	this->uploadGlBox();

	this->root.endVertices = _vertices.size();
	this->root.beginVertices = 0;

	this->maxVerticesPerQuad = _maxVerticesPerQuad;

	this->root.maxLeafBox = this->maxBoundingBox;
	this->root.minLeafBox = this->minBoundingBox;

	for (int i = 0; i < this->root.endVertices; i++) {
		this->vertexIndexList.push_back(i);
	}


	if (_vertices.size() < _maxVerticesPerQuad) {
		std::cout << "Model has less vertices then give max size per leaf. All informations stored in root." << std::endl;
		return;
	}

	this->vertexColorList.resize(_vertices.size());

	this->splitLeaf(this->root, _vertices);

	for (int i = 0; i < this->vertexColorList.size(); i++) {
		this->vertexColorList[i] = _colors[i];
	}

	this->uploadPointCloud(_vertices);
}

PC_OctreeSparse::~PC_OctreeSparse()
{
	glDeleteBuffers(3, vboBox);
}


void PC_OctreeSparse::getAabbUniforms(glm::mat4 & _modelMatrix)
{
	_modelMatrix = glm::mat4(1.0f);

	_modelMatrix = glm::translate(_modelMatrix, glm::vec3(minBoundingBox));

	glm::vec3 scaleVec = glm::abs(maxBoundingBox - minBoundingBox);

	_modelMatrix = glm::scale(_modelMatrix, scaleVec);
}

void PC_OctreeSparse::getAabbLeafUniforms(glm::mat4 & _modelMatrix, OctreeBox _leaf)
{
	_modelMatrix = glm::mat4(1.0f);

	_modelMatrix = glm::translate(_modelMatrix, glm::vec3(_leaf.minLeafBox));

	glm::vec3 scaleVec = glm::abs(_leaf.maxLeafBox - _leaf.minLeafBox);

	_modelMatrix = glm::scale(_modelMatrix, scaleVec);
}

void PC_OctreeSparse::drawBox()
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

void PC_OctreeSparse::uploadPointCloud(std::vector<glm::vec3>& _vertices, std::vector<glm::vec3>& _normals, std::vector<float>& _radius)
{
	glGenBuffers(5, vboPC);

	glBindBuffer(GL_ARRAY_BUFFER, vboPC[0]);
	glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(float) * 3, _vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vboPC[1]);
	glBufferData(GL_ARRAY_BUFFER, _normals.size() * sizeof(float) * 3, _normals.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vboPC[2]);
	glBufferData(GL_ARRAY_BUFFER, vertexColorList.size() * sizeof(float) * 3, vertexColorList.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vboPC[3]);
	glBufferData(GL_ARRAY_BUFFER, _radius.size() * sizeof(float), _radius.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vboPC[4]);
	glBufferData(GL_ARRAY_BUFFER, vertexIndexList.size() * sizeof(unsigned int), vertexIndexList.data(), GL_STATIC_DRAW);


}

void PC_OctreeSparse::uploadPointCloud(std::vector<glm::vec3>& _vertices)
{
	glGenBuffers(3, vboPC);

	glBindBuffer(GL_ARRAY_BUFFER, vboPC[0]);
	glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(float) * 3, _vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vboPC[1]);
	glBufferData(GL_ARRAY_BUFFER, vertexColorList.size() * sizeof(float) * 3, vertexColorList.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vboPC[2]);
	glBufferData(GL_ARRAY_BUFFER, vertexIndexList.size() * sizeof(unsigned int), vertexIndexList.data(), GL_STATIC_DRAW);


}

void PC_OctreeSparse::drawPointCloud()
{
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vboPC[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vboPC[1]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, vboPC[2]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, vboPC[3]);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboPC[4]);
	glDrawElements(GL_POINTS, vertexIndexList.size(), GL_UNSIGNED_INT, 0);
}

void PC_OctreeSparse::drawPointCloud_PosCol()
{
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vboPC[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vboPC[1]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboPC[2]);
	glDrawElements(GL_POINTS, vertexIndexList.size(), GL_UNSIGNED_INT, 0);
}

bool PC_OctreeSparse::onCorrectPlaneSide(glm::vec3& corner, glm::vec3& normal, glm::vec3& point) {
	if (glm::dot(normal, (corner - point)) > 0) {
		return true;
	}

	return false;
}


int PC_OctreeSparse::boxFrstrumCull(OctreeBox& leaf, glm::vec3& normal, glm::vec3& point) {
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

void PC_OctreeSparse::cullWithViewFrustrum(OctreeBox& leaf, viewFrustrum& vF)
{
	int partiallyInside = 0;
	int inOutTest = -1;

	//1
	inOutTest = boxFrstrumCull(leaf, vF.farNormal, vF.farPoint);
	if (inOutTest == 1) {
		//Box is compeltely outside of a plane, we can stop here
		return;
	}
	else if (inOutTest == 2) {
		//Box is partly inside the frustrum (if its not the leave yet keep splitting, else just take the entire box)
		if (leaf.children.size() == 0) {
			//this->addBoxToDraw(leaf, glm::vec3(1.0f, 0.0f, 0.0f));
			partiallyInside++;
		}
		else {
			for (int i = 0; i < leaf.children.size(); i++) {
				cullWithViewFrustrum(leaf.children[i], vF);
			}
			return;
		}
	}

	//2
	inOutTest = boxFrstrumCull(leaf, vF.nearNormal, vF.nearPoint);
	if (inOutTest == 1) {
		//Box is compeltely outside of a plane, we can stop here
		return;
	}
	else if (inOutTest == 2) {
		//Box is partly inside the frustrum (if its not the leave yet keep splitting, else just take the entire box)
		if (leaf.children.size() == 0) {
			//this->addBoxToDraw(leaf, glm::vec3(1.0f, 0.0f, 0.0f));
			partiallyInside++;
		}
		else {
			for (int i = 0; i < leaf.children.size(); i++) {
				cullWithViewFrustrum(leaf.children[i], vF);
			}
			return;
		}
	}

	//3
	inOutTest = boxFrstrumCull(leaf, vF.leftNormal, vF.leftPoint);
	if (inOutTest == 1) {
		//Box is compeltely outside of a plane, we can stop here
		return;
	}
	else if (inOutTest == 2) {
		//Box is partly inside the frustrum (if its not the leave yet keep splitting, else just take the entire box)
		if (leaf.children.size() == 0) {
			//this->addBoxToDraw(leaf, glm::vec3(1.0f, 0.0f, 0.0f));
			partiallyInside++;
		}
		else {
			for (int i = 0; i < leaf.children.size(); i++) {
				cullWithViewFrustrum(leaf.children[i], vF);
			}
			return;
		}
	}

	//4
	inOutTest = boxFrstrumCull(leaf, vF.rightNormal, vF.rightPoint);
	if (inOutTest == 1) {
		//Box is compeltely outside of a plane, we can stop here
		return;
	}
	else if (inOutTest == 2) {
		//Box is partly inside the frustrum (if its not the leave yet keep splitting, else just take the entire box)
		if (leaf.children.size() == 0) {
			//this->addBoxToDraw(leaf, glm::vec3(1.0f, 0.0f, 0.0f));
			partiallyInside++;
		}
		else {
			for (int i = 0; i < leaf.children.size(); i++) {
				cullWithViewFrustrum(leaf.children[i], vF);
			}
			return;
		}
	}


	//5
	inOutTest = boxFrstrumCull(leaf, vF.upNormal, vF.upPoint);
	if (inOutTest == 1) {
		//Box is compeltely outside of a plane, we can stop here
		return;
	}
	else if (inOutTest == 2) {
		//Box is partly inside the frustrum (if its not the leave yet keep splitting, else just take the entire box)
		if (leaf.children.size() == 0) {
			//this->addBoxToDraw(leaf, glm::vec3(1.0f, 0.0f, 0.0f));
			partiallyInside++;
		}
		else {
			for (int i = 0; i < leaf.children.size(); i++) {
				cullWithViewFrustrum(leaf.children[i], vF);
			}
			return;
		}
	}

	//6
	inOutTest = boxFrstrumCull(leaf, vF.downNormal, vF.downPoint);
	if (inOutTest == 1) {
		//Box is compeltely outside of a plane, we can stop here
		return;
	}
	else if (inOutTest == 2) {
		//Box is partly inside the frustrum (if its not the leave yet keep splitting, else just take the entire box)
		if (leaf.children.size() == 0) {
			//this->addBoxToDraw(leaf, glm::vec3(1.0f, 0.0f, 0.0f));
			partiallyInside++;
		}
		else {
			for (int i = 0; i < leaf.children.size(); i++) {
				cullWithViewFrustrum(leaf.children[i], vF);
			}
			return;
		}
	}

	//Box inside Frsutrum 
	this->addBoxToDraw(leaf, glm::vec3(0.0f, 0.0f, 1.0f));
	return;
}

void PC_OctreeSparse::addBoxToDraw(OctreeBox& leaf, glm::vec3 color) {
	glm::mat4 modelMatrix;
	this->getAabbLeafUniforms(modelMatrix, leaf);
	modelMatrixLowestLeaf.push_back(modelMatrix);

	leaf.boxColorId = colorLowestLeaf.size();
	colorLowestLeaf.push_back(color);
}

void PC_OctreeSparse::initViewFrustrumCull(OctreeBox& leaf, viewFrustrum& vF) {
	//std::cout << "initViewFrstrumCull: Start culling" << std::endl;
	this->modelMatrixLowestLeaf.clear();
	this->colorLowestLeaf.clear();

	this->cullWithViewFrustrum(leaf, vF);
}

void PC_OctreeSparse::drawPointCloudInFrustrumPrep(OctreeBox& leaf, viewFrustrum& vF)
{
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vboPC[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vboPC[1]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, vboPC[2]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, vboPC[3]);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboPC[4]);

	this->drawPointCloudInFrustrum(leaf, vF);
}

void PC_OctreeSparse::drawPointCloudInFrustrum(OctreeBox& leaf, viewFrustrum& vF)
{
	int partiallyInside = 0;
	int inOutTest = -1;

	//1
	inOutTest = boxFrstrumCull(leaf, vF.farNormal, vF.farPoint);
	if (inOutTest == 1) {
		//Box is compeltely outside of a plane, we can stop here
		return;
	}
	else if (inOutTest == 2) {
		//Box is partly inside the frustrum (if its not the leave yet keep splitting, else just take the entire box)
		if (leaf.children.size() == 0) {
			//this->addBoxToDraw(leaf, glm::vec3(1.0f, 0.0f, 0.0f));
			partiallyInside++;
		}
		else {
			for (int i = 0; i < leaf.children.size(); i++) {
				drawPointCloudInFrustrum(leaf.children[i], vF);
			}
			return;
		}
	}

	//2
	inOutTest = boxFrstrumCull(leaf, vF.nearNormal, vF.nearPoint);
	if (inOutTest == 1) {
		//Box is compeltely outside of a plane, we can stop here
		return;
	}
	else if (inOutTest == 2) {
		//Box is partly inside the frustrum (if its not the leave yet keep splitting, else just take the entire box)
		if (leaf.children.size() == 0) {
			//this->addBoxToDraw(leaf, glm::vec3(1.0f, 0.0f, 0.0f));
			partiallyInside++;
		}
		else {
			for (int i = 0; i < leaf.children.size(); i++) {
				drawPointCloudInFrustrum(leaf.children[i], vF);
			}
			return;
		}
	}

	//3
	inOutTest = boxFrstrumCull(leaf, vF.leftNormal, vF.leftPoint);
	if (inOutTest == 1) {
		//Box is compeltely outside of a plane, we can stop here
		return;
	}
	else if (inOutTest == 2) {
		//Box is partly inside the frustrum (if its not the leave yet keep splitting, else just take the entire box)
		if (leaf.children.size() == 0) {
			//this->addBoxToDraw(leaf, glm::vec3(1.0f, 0.0f, 0.0f));
			partiallyInside++;
		}
		else {
			for (int i = 0; i < leaf.children.size(); i++) {
				drawPointCloudInFrustrum(leaf.children[i], vF);
			}
			return;
		}
	}

	//4
	inOutTest = boxFrstrumCull(leaf, vF.rightNormal, vF.rightPoint);
	if (inOutTest == 1) {
		//Box is compeltely outside of a plane, we can stop here
		return;
	}
	else if (inOutTest == 2) {
		//Box is partly inside the frustrum (if its not the leave yet keep splitting, else just take the entire box)
		if (leaf.children.size() == 0) {
			//this->addBoxToDraw(leaf, glm::vec3(1.0f, 0.0f, 0.0f));
			partiallyInside++;
		}
		else {
			for (int i = 0; i < leaf.children.size(); i++) {
				drawPointCloudInFrustrum(leaf.children[i], vF);
			}
			return;
		}
	}


	//5
	inOutTest = boxFrstrumCull(leaf, vF.upNormal, vF.upPoint);
	if (inOutTest == 1) {
		//Box is compeltely outside of a plane, we can stop here
		return;
	}
	else if (inOutTest == 2) {
		//Box is partly inside the frustrum (if its not the leave yet keep splitting, else just take the entire box)
		if (leaf.children.size() == 0) {
			//this->addBoxToDraw(leaf, glm::vec3(1.0f, 0.0f, 0.0f));
			partiallyInside++;
		}
		else {
			for (int i = 0; i < leaf.children.size(); i++) {
				drawPointCloudInFrustrum(leaf.children[i], vF);
			}
			return;
		}
	}

	//6
	inOutTest = boxFrstrumCull(leaf, vF.downNormal, vF.downPoint);
	if (inOutTest == 1) {
		//Box is compeltely outside of a plane, we can stop here
		return;
	}
	else if (inOutTest == 2) {
		//Box is partly inside the frustrum (if its not the leave yet keep splitting, else just take the entire box)
		if (leaf.children.size() == 0) {
			//this->addBoxToDraw(leaf, glm::vec3(1.0f, 0.0f, 0.0f));
			partiallyInside++;
		}
		else {
			for (int i = 0; i < leaf.children.size(); i++) {
				drawPointCloudInFrustrum(leaf.children[i], vF);
			}
			return;
		}
	}

	//Box inside Frsutrum 
	//std::cout << leaf.beginVertices << " " << leaf.endVertices << ", " << leaf.endVertices - leaf.beginVertices << std::endl;
	//if(leaf.endVertices - leaf.beginVertices <= 1){
	//	return;
	//}

	glDrawElements(GL_POINTS, leaf.endVertices - leaf.beginVertices, GL_UNSIGNED_INT, (void*)(leaf.beginVertices * sizeof(GLuint)));
	//glDrawArrays(GL_POINTS, leaf.beginVertices, leaf.endVertices - leaf.beginVertices);  //2nd Argument: begin of array , 3rd Arguemnt: How many to render (NOT up to which index!!!)
	return;
}

/* *********************************************************************************************************
//Functions (private)
********************************************************************************************************* */

/********************************		OCTREE creation ****************************************************
Names of new octree-Boxes
/----------------/|
/-------/--------/ |
/_______/________/| |
|		|		| | |
|000	|010	| |/|
|		|		| / |
-----------------/| |
|		|		| | |
|100	|110	| |/
|		|		| /
-----------------/

Coordinate System (OpenGL standard)
y-Direction
|
|  / z-Direction
| /
|/
-------- x-Direction

Octree-Split-Names-Convention, the new boxes in z-Direction are labeled 001, 011,...

*/



void PC_OctreeSparse::splitLeaf(OctreeBox& leaf, std::vector<glm::vec3>& _vertices) {
	//std::cout << " -- Starting Split from " <<leaf.beginVertices <<" to " << leaf.endVertices << "--" << std::endl;

	{
		if (leaf.endVertices - leaf.beginVertices < this->maxVerticesPerQuad) {

			if (leaf.endVertices - leaf.beginVertices == 0) {
				std::cout << "Error: Empty Box!" << std::endl;
			}

			glm::mat4 modelMatrix;
			this->getAabbLeafUniforms(modelMatrix, leaf);
			modelMatrixLowestLeaf.push_back(modelMatrix);

			leaf.boxColorId = colorLowestLeaf.size();
			colorLowestLeaf.push_back(vertexColorList[this->vertexIndexList[leaf.beginVertices]]);


			return;
		}

		std::vector<int> newLeaf000, newLeaf010, newLeaf100, newLeaf110, newLeaf001, newLeaf011, newLeaf101, newLeaf111;
		glm::vec3 splittingPlain = leaf.minLeafBox + 0.5f * (leaf.maxLeafBox - leaf.minLeafBox);

		glm::vec3 splitDiagonal = leaf.maxLeafBox - leaf.minLeafBox;


		for (int j = leaf.beginVertices; j < leaf.endVertices; j++) {

			glm::vec3 currentVertex = _vertices[this->vertexIndexList[j]];
			int i = this->vertexIndexList[j];

			if (currentVertex.x <= splittingPlain.x && currentVertex.y > splittingPlain.y && currentVertex.z <= splittingPlain.z) {
				newLeaf000.push_back(i);
				this->vertexColorList[i] = this->colorOctree[0];
			}

			else if (currentVertex.x <= splittingPlain.x && currentVertex.y <= splittingPlain.y && currentVertex.z <= splittingPlain.z) {
				newLeaf100.push_back(i);
				this->vertexColorList[i] = this->colorOctree[1];
			}

			else if (currentVertex.x > splittingPlain.x && currentVertex.y > splittingPlain.y && currentVertex.z <= splittingPlain.z) {
				newLeaf010.push_back(i);
				this->vertexColorList[i] = this->colorOctree[2];
			}

			else if (currentVertex.x > splittingPlain.x && currentVertex.y <= splittingPlain.y && currentVertex.z <= splittingPlain.z) {
				newLeaf110.push_back(i);
				this->vertexColorList[i] = this->colorOctree[3];
			}

			else if (currentVertex.x <= splittingPlain.x && currentVertex.y > splittingPlain.y && currentVertex.z > splittingPlain.z) {
				newLeaf001.push_back(i);
				this->vertexColorList[i] = this->colorOctree[4];
			}

			else if (currentVertex.x <= splittingPlain.x && currentVertex.y <= splittingPlain.y && currentVertex.z > splittingPlain.z) {
				newLeaf101.push_back(i);
				this->vertexColorList[i] = this->colorOctree[5];
			}

			else if (currentVertex.x > splittingPlain.x && currentVertex.y > splittingPlain.y && currentVertex.z > splittingPlain.z) {
				newLeaf011.push_back(i);
				this->vertexColorList[i] = this->colorOctree[6];
			}

			else if (currentVertex.x > splittingPlain.x && currentVertex.y <= splittingPlain.y && currentVertex.z > splittingPlain.z) {
				newLeaf111.push_back(i);
				this->vertexColorList[i] = this->colorOctree[7];
			}
			else {
				std::cout << "ERROR: PC_Octree.cpp could not place vertex into octree" << std::endl;
			}
		}

		int currentStart = leaf.beginVertices;

		//Front
		if (newLeaf000.size() > 0) {
			leaf.bitMaskChildren[0] = 1;
			glm::vec3 min, max;

			min = leaf.minLeafBox + glm::vec3(0.0f, 0.5f * splitDiagonal.y, 0.0f);
			max = leaf.minLeafBox + glm::vec3(0.5f * splitDiagonal.x, splitDiagonal.y, 0.5f * splitDiagonal.z);
			//this->getAABB(min, max, _vertices, newLeaf000);

			leaf.children.push_back(OctreeBox(currentStart, currentStart + newLeaf000.size(), min, max));

			//std::cout << "currentStart: " << currentStart << ", currentEnd: " << currentStart + newLeaf000.size() << std::endl;

			//std::cout << "Start Copy newLeaf000" << std::endl;
			//std::copy(newLeaf000.begin(), newLeaf000.end(), vertexIndexList.begin() + leaf.beginVertices + currentStart);
			this->copyIndexVector(newLeaf000, currentStart);  //this->copyIndexVector(newLeaf000, leaf.beginVertices + currentStart);
															  //std::cout << "End Copy" << std::endl;
			currentStart += newLeaf000.size();
		}

		if (newLeaf010.size() > 0) {
			leaf.bitMaskChildren[1] = 1;
			glm::vec3 min, max;

			min = leaf.minLeafBox + glm::vec3(0.5f * splitDiagonal.x, 0.5f * splitDiagonal.y, 0.0f);
			max = leaf.minLeafBox + glm::vec3(splitDiagonal.x, splitDiagonal.y, 0.5f * splitDiagonal.z);

			//min = leaf.minLeafBox;
			//max = leaf.minLeafBox + glm::vec3(0.5f * splitDiagonal.x, 0.5f * splitDiagonal.y, 0.5f * splitDiagonal.z);

			//this->getAABB(min, max, _vertices, newLeaf010);

			leaf.children.push_back(OctreeBox(currentStart, currentStart + newLeaf010.size(), min, max));

			//std::cout << "currentStart: " << currentStart << ", currentEnd: " << currentStart + newLeaf010.size() << std::endl;
			//std::cout << "Start Copy newLeaf010" << std::endl;
			//std::copy(newLeaf010.begin(), newLeaf010.end(), vertexIndexList.begin() + leaf.beginVertices + currentStart);
			this->copyIndexVector(newLeaf010, currentStart);
			//std::cout << "End Copy" << std::endl;
			currentStart += newLeaf010.size();
		}

		if (newLeaf100.size() > 0) {
			leaf.bitMaskChildren[2] = 1;
			glm::vec3 min, max;

			min = leaf.minLeafBox;
			max = leaf.minLeafBox + glm::vec3(0.5f * splitDiagonal.x, 0.5f * splitDiagonal.y, 0.5f * splitDiagonal.z);
			//min = leaf.minLeafBox + glm::vec3(0.5f * splitDiagonal.x, 0.5f * splitDiagonal.y, 0.0f);
			//max = leaf.minLeafBox + glm::vec3(splitDiagonal.x, splitDiagonal.y, 0.5f * splitDiagonal.z);
			//this->getAABB(min, max, _vertices, newLeaf100);

			leaf.children.push_back(OctreeBox(currentStart, currentStart + newLeaf100.size(), min, max));

			//std::cout << "currentStart: " << currentStart << ", currentEnd: " << currentStart + newLeaf100.size() << std::endl;
			//std::cout << "Start Copy newLeaf100" << std::endl;
			//std::copy(newLeaf100.begin(), newLeaf100.end(), vertexIndexList.begin() + leaf.beginVertices + currentStart);
			this->copyIndexVector(newLeaf100, currentStart);
			//std::cout << "End Copy" << std::endl;
			currentStart += newLeaf100.size();
		}

		if (newLeaf110.size() > 0) {
			leaf.bitMaskChildren[3] = 1;
			glm::vec3 min, max;

			min = leaf.minLeafBox + glm::vec3(0.5f * splitDiagonal.x, 0.0f, 0.0f);
			max = leaf.minLeafBox + glm::vec3(splitDiagonal.x, 0.5f * splitDiagonal.y, 0.5f * splitDiagonal.z);
			//this->getAABB(min, max, _vertices, newLeaf110);

			leaf.children.push_back(OctreeBox(currentStart, currentStart + newLeaf110.size(), min, max));

			//std::cout << "currentStart: " << currentStart << ", currentEnd: " << currentStart + newLeaf110.size() << std::endl;
			//std::cout << "Start Copy newLeaf110" << std::endl;
			//std::copy(newLeaf110.begin(), newLeaf110.end(), vertexIndexList.begin() + leaf.beginVertices + currentStart);
			this->copyIndexVector(newLeaf110, currentStart);
			//std::cout << "End Copy" << std::endl;
			currentStart += newLeaf110.size();
		}

		//Back
		if (newLeaf001.size() > 0) {
			leaf.bitMaskChildren[4] = 1;
			glm::vec3 min, max;

			min = leaf.minLeafBox + glm::vec3(0.0f, 0.5f * splitDiagonal.y, 0.5f * splitDiagonal.z);
			max = leaf.minLeafBox + glm::vec3(0.5f * splitDiagonal.x, splitDiagonal.y, splitDiagonal.z);
			//this->getAABB(min, max, _vertices, newLeaf000);

			leaf.children.push_back(OctreeBox(currentStart, currentStart + newLeaf001.size(), min, max));

			//std::cout << "currentStart: " << currentStart << ", currentEnd: " << currentStart + newLeaf001.size() << std::endl;
			//std::cout << "Start Copy newLeaf001" << std::endl;
			//std::copy(newLeaf001.begin(), newLeaf001.end(), vertexIndexList.begin() + leaf.beginVertices + currentStart);
			this->copyIndexVector(newLeaf001, currentStart);
			//std::cout << "End Copy" << std::endl;
			currentStart += newLeaf001.size();
		}

		if (newLeaf011.size() > 0) {
			leaf.bitMaskChildren[5] = 1;
			glm::vec3 min, max;

			min = leaf.minLeafBox + glm::vec3(0.5f * splitDiagonal.x, 0.5f * splitDiagonal.y, 0.5f * splitDiagonal.z);
			max = leaf.minLeafBox + glm::vec3(splitDiagonal.x, splitDiagonal.y, splitDiagonal.z);

			//min = leaf.minLeafBox + glm::vec3(0.0f, 0.0f, 0.5f * splitDiagonal.z);
			//max = leaf.minLeafBox + glm::vec3(0.5f * splitDiagonal.x, 0.5f * splitDiagonal.y, splitDiagonal.z);
			//this->getAABB(min, max, _vertices, newLeaf010);

			leaf.children.push_back(OctreeBox(currentStart, currentStart + newLeaf011.size(), min, max));

			//std::cout << "currentStart: " << currentStart << ", currentEnd: " << currentStart + newLeaf011.size() << std::endl;
			//std::cout << "Start Copy newLeaf011" << std::endl;
			//std::copy(newLeaf011.begin(), newLeaf011.end(), vertexIndexList.begin() + leaf.beginVertices + currentStart);
			this->copyIndexVector(newLeaf011, currentStart);
			//std::cout << "End Copy" << std::endl;
			currentStart += newLeaf011.size();
		}

		if (newLeaf101.size() > 0) {
			leaf.bitMaskChildren[6] = 1;
			glm::vec3 min, max;

			min = leaf.minLeafBox + glm::vec3(0.0f, 0.0f, 0.5f * splitDiagonal.z);;
			max = leaf.minLeafBox + glm::vec3(0.5f * splitDiagonal.x, 0.5f * splitDiagonal.y, splitDiagonal.z);

			//min = leaf.minLeafBox + glm::vec3(0.5f * splitDiagonal.x, 0.5f * splitDiagonal.y, 0.5f * splitDiagonal.z);
			//max = leaf.minLeafBox + glm::vec3(splitDiagonal.x, splitDiagonal.y, splitDiagonal.z);

			leaf.children.push_back(OctreeBox(currentStart, currentStart + newLeaf101.size(), min, max));

			//std::cout << "currentStart: " << currentStart << ", currentEnd: " << currentStart + newLeaf101.size() << std::endl;
			//std::cout << "Start Copy newLeaf101" << std::endl;
			//std::copy(newLeaf101.begin(), newLeaf101.end(), vertexIndexList.begin() + leaf.beginVertices + currentStart);
			this->copyIndexVector(newLeaf101, currentStart);
			//std::cout << "End Copy" << std::endl;
			currentStart += newLeaf101.size();
		}

		if (newLeaf111.size() > 0) {
			leaf.bitMaskChildren[7] = 1;
			glm::vec3 min, max;

			min = leaf.minLeafBox + glm::vec3(0.5f * splitDiagonal.x, 0.0f, 0.5f * splitDiagonal.z);
			max = leaf.minLeafBox + glm::vec3(splitDiagonal.x, 0.5f * splitDiagonal.y, splitDiagonal.z);
			//this->getAABB(min, max, _vertices, newLeaf110);

			leaf.children.push_back(OctreeBox(currentStart, currentStart + newLeaf111.size(), min, max));

			//std::cout << "currentStart: " << currentStart << ", currentEnd: " << currentStart + newLeaf111.size() << std::endl;
			//std::cout << "Start Copy newLeaf111" << std::endl;
			//std::copy(newLeaf111.begin(), newLeaf111.end(), vertexIndexList.begin() + leaf.beginVertices + currentStart);
			this->copyIndexVector(newLeaf111, currentStart);
			//std::cout << "End Copy" << std::endl;
			currentStart += newLeaf111.size();
		}
	}

	//std::cout << " -- Finished Split --" << std::endl;

	//Debug Model Matrix
	/*if (leaf.bitMaskChildren[0] == 0 && leaf.bitMaskChildren[1] == 0 && leaf.bitMaskChildren[2] == 0 && leaf.bitMaskChildren[3] == 0 && leaf.bitMaskChildren[4] == 0 && leaf.bitMaskChildren[5] == 0 && leaf.bitMaskChildren[6] == 0 && leaf.bitMaskChildren[7] == 0) {
	glm::mat4 modelMatrix;
	this->getAabbLeafUniforms(modelMatrix, leaf);
	modelMatrixLowestLeaf.push_back(modelMatrix);
	}*/

	/*
	DebugMe: This will thow a stack overflow due to low stack size
	*/

	//std::cout << " leaf.children.size(): " << leaf.children.size() << std::endl;

	for (int i = 0; i < leaf.children.size(); i++) {

		//std::cout << "Split if: "<< leaf.children[i].endVertices - leaf.children[i].beginVertices << " > " << this->maxVerticesPerQuad << std::endl;

		//if (leaf.children[i].endVertices - leaf.children[i].beginVertices > this->maxVerticesPerQuad) {
		this->splitLeaf(leaf.children[i], _vertices);
		//}
	}
}

void PC_OctreeSparse::copyIndexVector(std::vector<int>& leafIndex, int offset) {
	for (int i = 0; i < leafIndex.size(); i++) {
		//std::cout << "i: " << i << std::endl;
		//std::cout << "i + offset: " << i + offset << std::endl;
		//std::cout << "offset: " << offset << std::endl;
		//std::cout << "this->vertexIndexList.size(): " << this->vertexIndexList.size() << std::endl;
		//std::cout << "leafIndex.size(): " << leafIndex.size() << std::endl;
		//std::cout << "---" << std::endl;
		this->vertexIndexList[i + offset] = leafIndex[i];
	}
}

void PC_OctreeSparse::getAABB(glm::vec3& min, glm::vec3& max, std::vector<glm::vec3>& _vertices)
{
	min = _vertices[0];
	max = _vertices[0];

	for (int i = 1; i < _vertices.size(); i++) {

		if (_vertices[i].x < min.x) {
			min.x = _vertices[i].x;
		}

		if (_vertices[i].y < min.y) {
			min.y = _vertices[i].y;
		}

		if (_vertices[i].z < min.z) {
			min.z = _vertices[i].z;
		}

		if (_vertices[i].x > max.x) {
			max.x = _vertices[i].x;
		}

		if (_vertices[i].y > max.y) {
			max.y = _vertices[i].y;
		}

		if (_vertices[i].z > max.z) {
			max.z = _vertices[i].z;
		}

	}
}

void PC_OctreeSparse::getAABB(glm::vec3& min, glm::vec3& max, std::vector<glm::vec3>& _vertices, std::vector<int> _indices)
{
	min = _vertices[_indices[0]];
	max = _vertices[_indices[0]];

	for (int i = 1; i < _indices.size(); i++) {

		if (_vertices[_indices[i]].x < min.x) {
			min.x = _vertices[_indices[i]].x;
		}

		if (_vertices[_indices[i]].y < min.y) {
			min.y = _vertices[_indices[i]].y;
		}

		if (_vertices[_indices[i]].z < min.z) {
			min.z = _vertices[_indices[i]].z;
		}

		if (_vertices[_indices[i]].x > max.x) {
			max.x = _vertices[_indices[i]].x;
		}

		if (_vertices[_indices[i]].y > max.y) {
			max.y = _vertices[_indices[i]].y;
		}

		if (_vertices[_indices[i]].z > max.z) {
			max.z = _vertices[_indices[i]].z;
		}

	}
}

void PC_OctreeSparse::uploadGlBox()
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


