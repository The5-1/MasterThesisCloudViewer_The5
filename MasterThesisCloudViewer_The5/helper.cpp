/*
_CRT_SECURE_NO_DEPRECATE allows the use of fopen, _ftime, ...
*/
#define _CRT_SECURE_NO_DEPRECATE

#include <cstring>
#include <utility>
#include <map>
#include "times.h"
#include <glm/gtc/type_ptr.hpp>
#include <Ant/AntTweakBar.h>

#include "helper.h"
#include "loadPNG.h"

#include "mathConstants.h"

//Camera-System
extern cameraSystem cam;

//Camera-Matrix
//extern glm::vec3 lightDir;
extern glm::mat4 projMatrix;
extern glm::mat4 viewMatrix;

//Mouse - Detection
extern bool leftMouseClick;
extern int leftMouseClickX;
extern int leftMouseClickY;

//glut stuff
void keyboard(unsigned char key, int x, int y) {
	if (TwEventKeyboardGLUT(key, x, y)) {
		glutPostRedisplay();
		return;
	}

	//if we modfy the parameters inside the GLUT keyPressed function like here...
	//we can only do 1 key press at a time
	//we would actually need to just buffer the inputs here and then feed our own camera update method with what happened
	//http://www.swiftless.com/tutorials/opengl/keyboard.html
	if (key == 'w') cam.moveForward(cam.delta);		
	if (key == 's') cam.moveBackward(cam.delta);
	if (key == 'a') cam.moveLeft(cam.delta);
	if (key == 'd') cam.moveRight(cam.delta);
	if (key == 'q') cam.roll(0.01f*cam.mouseDelta);
	if (key == 'e') cam.roll(0.01f*-cam.mouseDelta);
	//updateCamera();
	cam.Update();

	switch(key) {

	case 27:
		std::cout << "exit" << std::endl;
		exit(0);
		break;
	case 'r':
		loadShader(false);
		break;
	case 'c':
		const glm::vec4& d = cam.viewDir;
		const glm::vec4& u = cam.upDir;
		const glm::vec4& p = cam.position;
		std::cout << "cam-dir: " << d.x << ", " << d.y << ", " << d.z << std::endl <<
		             "cam-up:  " << u.x << ", " << u.y << ", " << u.z << std::endl <<
					 "cam-pos: " << p.x << ", " << p.y << ", " << p.z << std::endl;
		break;
	}
	glutPostRedisplay();
}

void onMouseDown(int button, int state, int x, int y) {
	/**************************************************
	Button:		0	Left-MouseButton
				1	Mouse-Wheel (Press)
				2	Right-MouseButton
				3	Mouse - Wheel(Forward)
				4	Mouse - Wheel(Backwards)
	
	State:	GLUT_DOWN and GLUT_UP

	x,y:	Current coordinates of the mouse ( 0 <= x <= WIDTH,  0 <= x <= HEIGHT)
			Left side of the screen x = 0
			Upper side of the screen y = 0
			To convert them to normalized coordinates:	xNomralized = 2 * ( x / WIDTH ) - 1
														yNomralized = -2 * ( y / HEIGHT ) + 1
														zNomralized = 1
	**************************************************/

	/**************************************************
	Mouse Picking with ray tracing:
	http://antongerdelan.net/opengl/raycasting.html

	Intersection-Test:
	http://www.lighthouse3d.com/tutorials/maths/ray-triangle-intersection/
	**************************************************/

	if (TwEventMouseButtonGLUT(button, state, x, y)) {
		glutPostRedisplay();
		return;
	}

	/*TO_DO:
	Even if we deactivate this holding any mouse button down and moving the mouse will result in camera movement.
	WHY???
	*/

	switch (button) {
	case 4: //Mouse - Wheel(Backwards)
		cam.delta *= 0.75f;
		break;
	case 3: //Mouse - Wheel(Forward)
		cam.delta *= (4.f / 3.f);
		break;
	case 2: //Right-MouseButton
		if (state == GLUT_DOWN) {
			cam.rightMouseDown = true;
		}
		else {
			cam.rightMouseDown = false;
		}
		break;
	case 1: //Mouse-Wheel (Press)
		if (state == GLUT_DOWN) {
			cam.middleMouseDown = true;
		}
		else{
			cam.middleMouseDown = false;
		}
		break;

	case 0: //Left-MouseButton
		if (state == GLUT_DOWN) {
			leftMouseClickX = x;
			leftMouseClickY = y;
			leftMouseClick = true;
		}
		else {
		}
		break;
	}

	cam.currentX = x;
	cam.currentY = y;
}

void onMouseMove(int x, int y) {
	if (TwEventMouseMotionGLUT(x, y)) {
		glutPostRedisplay();
		return;
	}

	const float dx = (cam.currentX-x);
	const float dy = (cam.currentY-y);

	if (cam.rightMouseDown) {
		cam.moveUp(-dy*0.03f);
		cam.moveRight(dx*0.03f);
	}
	else if (cam.middleMouseDown) {
		cam.moveForward(dy*0.1f*cam.mouseDelta);
		cam.roll(dx*0.001f*cam.mouseDelta);
	}
	else { //left mouse
		cam.yaw(dx*0.002f*cam.mouseDelta);
		cam.pitch(dy*0.003f*cam.mouseDelta);
	}


	cam.currentX = x;
	cam.currentY = y;

	//updateCamera();
	cam.Update();
}

void onIdle() {
	//ToDo: What does this do here?
	//glutPostRedisplay();
}

void initGL() {
	gl_check_error("before init GL");
	glClearDepth(1);
	glClearColor(0.1, 0.4, 1.0, 1.0);
	glEnable(GL_DEPTH_TEST); // turn on the depth test

	loadShader(true);
	gl_check_error("shader built");

	const glm::vec3 eye =    glm::vec3(cam.position);
	const glm::vec3 center = glm::vec3(cam.position + cam.viewDir);
	const glm::vec3 up =     glm::vec3(cam.upDir);

	viewMatrix = glm::lookAt(eye,center,up);
	float fov = 70.0f;
	GLfloat aspect = (GLfloat)WIDTH / (GLfloat)HEIGHT;
	float nearP = 1.0f, farP = 500.0f;
	projMatrix =  glm::perspective(fov, aspect, nearP, farP);

	cam.Hnear = 2.0f * glm::tan(fov / 2.0f) * nearP;
	cam.Wnear = cam.Hnear * aspect;

	cam.Hfar = 2.0f * tan(fov / 2.0f) * farP;
	cam.Wfar = cam.Hfar * aspect;

	cam.nearDist = nearP;
	cam.farDist = farP;

	reshape(WIDTH, HEIGHT);
}

/*
void updateCamera() {
	const glm::vec3 eye =    glm::vec3(cam.position);
	const glm::vec3 center = glm::vec3(cam.position + cam.viewDir);
	const glm::vec3 up =     glm::vec3(cam.upDir);
	viewMatrix = glm::lookAt(eye,center,up);
	glutPostRedisplay();
}
*/


void reshape(int w, int h) {
	static bool initialization= true;

	if (initialization) {
		glViewport(0, 0,(GLsizei)w, (GLsizei)h);
		projMatrix = glm::perspective(70.0f, (GLfloat)w / (GLfloat)h, 1.0f, 100.0f);
		initialization = false;
	}

	TwWindowSize(w,h);
}

bool _gl_check_error(const char* arg, const char *file, int line, const char *function)
{
	GLenum err = glGetError();

	//while (err != GL_NO_ERROR) { //since we exit in the first while-loop, why even while?
	if (err != GL_NO_ERROR) {	
		string error;

		switch (err) {
			case GL_INVALID_OPERATION:      error = "INVALID_OPERATION";      break;
			case GL_INVALID_ENUM:           error = "INVALID_ENUM";           break;
			case GL_INVALID_VALUE:          error = "INVALID_VALUE";          break;
			case GL_OUT_OF_MEMORY:          error = "OUT_OF_MEMORY";          break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:  error = "INVALID_FRAMEBUFFER_OPERATION";  break;
		}
		std::cerr << "ERROR tag: " << "\"" << arg << "\": " << std::endl;
		std::cerr << "\t GLEW error: " << glewGetErrorString(err) << "(" << err << ")" << std::endl;
		std::cerr << "\t GL error: " << error.c_str() << endl;
		std::cerr << "\t file: " << file << " line: " << line << " function: " << function << endl;
		std::exit(1); //this EXITS the program ---> why would we want that
	}
	return err == 0;
};

static string textFileRead(const char *fileName) 
{
	string fileString;
	string line;
	
	ifstream file(fileName,ios_base::in);

	if (file.is_open()) 
	{
		while (!file.eof()) 
		{
			getline(file, line);
		  	fileString.append(line);
			fileString.append("\n");
		}
		file.close();
	}
	else
		cout<<"Unable to open "<<fileName<<"\n";

    return fileString;
}

glm::vec3 HSVtoRGB(float  fH, float  fS, float  fV)
{
	float fC = fV * fS; // Chroma
	float fHPrime = fmod(fH / 60.0, 6);
	float fX = fC * (1 - fabs(fmod(fHPrime, 2) - 1));
	float fM = fV - fC;

	glm::vec3 color;

	if (0 <= fHPrime && fHPrime < 1) {
		color.x = fC;
		color.y = fX;
		color.z = 0;
	}
	else if (1 <= fHPrime && fHPrime < 2) {
		color.x = fX;
		color.y = fC;
		color.z = 0;
	}
	else if (2 <= fHPrime && fHPrime < 3) {
		color.x = 0;
		color.y = fC;
		color.z = fX;
	}
	else if (3 <= fHPrime && fHPrime < 4) {
		color.x = 0;
		color.y = fX;
		color.z = fC;
	}
	else if (4 <= fHPrime && fHPrime < 5) {
		color.x = fX;
		color.y = 0;
		color.z = fC;
	}
	else if (5 <= fHPrime && fHPrime < 6) {
		color.x = fC;
		color.y = 0;
		color.z = fX;
	}
	else {
		color.x = 0;
		color.y = 0;
		color.z = 0;
	}

	color.x += fM;
	color.y += fM;
	color.z += fM;

	return color;
}

// screen space quad
void simpleQuad::upload() {
	vertices = vector<glm::vec3>(4);
	vertices = { glm::vec3(-1, -1, 0),
		         glm::vec3( 1, -1, 0),
				 glm::vec3( 1,  1, 0),
				 glm::vec3(-1,  1, 0) };

	indices = vector<unsigned int>(6);
	indices = { 0, 1, 3, 1, 2, 3 };
	

	glGenBuffers(2, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float)*3, vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
}

void simpleQuad::draw() {
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

void simpleCube::upload() {
	//vertices = vector<glm::vec3>(4);
	vertices = { glm::vec3(0.0,  0.0,  0.0),
					glm::vec3(0.0,  0.0,  1.0),
					glm::vec3(0.0,  1.0,  0.0),
					glm::vec3(0.0,  1.0,  1.0),
					glm::vec3(1.0,  0.0,  0.0),
					glm::vec3(1.0,  0.0,  1.0),
					glm::vec3(1.0,  1.0,  0.0),
					glm::vec3(1.0,  1.0,  1.0) };

	//indices = vector<unsigned int>(6);
	indices = { 1,7,5, 1,3,7, 1,4,3, 1,2,4, 3,8,7, 3,4,8, 5,7,8, 5,8,6, 1,5,6, 1,6,2, 2,6,8, 2,8,4};


	glGenBuffers(2, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) * 3, vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
}

void simpleCube::draw() {
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

// simple Model
simpleModel::simpleModel () {};

//simpleModel::simpleModel(const char *filename) {
//	std::vector<glm::vec3> normals_tmp;
//	std::vector<unsigned int> nIndices;
//
//	FILE * file = fopen(filename, "r");
//	if (file == NULL) {
//		cerr << "Model file not found: " << filename << endl;
//		exit(0);
//	}
//
//	while (1)
//	{
//		char lineHeader[128];
//		int res = fscanf(file, "%s", lineHeader);
//		if (res == EOF)
//			break;
//
//		if (strcmp(lineHeader, "v") == 0)
//		{
//			glm::vec3 vertex;
//			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
//			vertices.push_back(vertex);
//		}
//		else if (strcmp(lineHeader, "vn") == 0)
//		{
//			glm::vec3 normal;
//			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
//			normals_tmp.push_back(normal);
//		}
//		else if (strcmp(lineHeader, "f") == 0)
//		{
//			unsigned int vIndex[3], uvIndex[3], nIndex[3];
//			int matches = fscanf(file, "%d//%d %d//%d %d//%d\n", &vIndex[0], &nIndex[0],
//				&vIndex[1], &nIndex[1],
//				&vIndex[2], &nIndex[2]);
//			if (matches < 6) {
//				cerr << "Information missin in obj file. " << matches << ", " << lineHeader << endl;
//				exit(0);
//			}
//
//
//			for (int i = 0; i < 3; ++i) {
//				indices.push_back(vIndex[i] - 1);
//				nIndices.push_back(nIndex[i] - 1);
//			}
//		}
//	}
//
//	//sort normals
//	for (int i = 0; i < indices.size(); ++i) {
//
//	}
//
//
//	//normals = normals_tmp;
//	/*std::cout << "V " << vertices.size() << std::endl;
//	std::cout << "N " << normals.size() << std::endl;*/
//
//	/*std::vector<glm::vec3> extra_verts;
//	std::vector<glm::vec3> extra_norms;
//	std::map<std::pair<int, int>, int> mapping;
//
//	normals.resize(vertices.size());
//	std::vector<bool> tested(vertices.size(), false);
//
//	for (int i = 0; i < indices.size(); ++i) {
//	int vIdx = indices[i];
//	int nIdx = nIndices[i];
//
//	if (tested[vIdx] && vIdx != nIdx ) {
//	auto it = mapping.find(std::pair<int, int>(vIdx, nIdx));
//	if (it != mapping.end())
//	indices[i] = vertices.size() + it->second -1;
//	else {
//	extra_verts.push_back(vertices[indices[i]]);
//	extra_norms.push_back(normals_tmp[nIndices[i]]);
//	indices[i] = vertices.size() + extra_verts.size() -1;
//	mapping[std::pair<int, int>(vIdx, nIdx)] = extra_verts.size();
//	}
//	}
//	else {
//	normals[indices[i]] = normals_tmp[nIndices[i]];
//	tested[indices[i]] = true;
//	}
//	}
//
//	for (auto it = tested.begin(); it != tested.end(); ++it)
//	if (*it != true)
//	cout << "probs" << endl;
//
//
//	vertices.insert(vertices.end(), extra_verts.begin(), extra_verts.end());
//	normals.insert(normals.end(), extra_norms.begin(), extra_norms.end());
//
//	mapping.clear();
//	extra_verts.clear();
//	extra_norms.clear();
//	nIndices.clear();
//	normals_tmp.clear();*/
//
//
//	if (vertices.size() != normals.size())
//	{
//		cerr << "Object data (vertices/normals) incensitent." << endl;
//		exit(0);
//	}
//	else
//	{
//		cout << "model loaded: " << filename << endl;
//	}
//
//}

simpleModel::simpleModel (const char *filename, bool improveMesh) {
	std::vector<glm::vec3> normals_tmp;
	std::vector<unsigned int> nIndices;

	FILE * file = fopen(filename, "r");
	if (file == NULL) {
		cerr << "Model file not found: " << filename << endl;
		exit(0);
	}

	while (1) 
	{
		char lineHeader[128];
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break;

		if (strcmp (lineHeader, "v") == 0) 
		{
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			vertices.push_back(vertex);
		}
		else if (strcmp (lineHeader, "vn") == 0) 
		{
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			if (improveMesh) {
				normals_tmp.push_back(normal);
			}
			else {
				normals.push_back(normal);
			}
		} 
		else if (strcmp (lineHeader, "f") == 0) 
		{
			unsigned int vIndex[3], uvIndex[3], nIndex[3];
			int matches = fscanf(file, "%d//%d %d//%d %d//%d\n", &vIndex[0], &nIndex[0],
								                                       &vIndex[1], &nIndex[1],
								                                       &vIndex[2], &nIndex[2]);
			if (matches < 6)  {
				cerr << "Information missin in obj file. " << matches  << ", " << lineHeader<< endl;
				exit(0);
			}


			for (int i = 0; i < 3; ++i)  {
				indices.push_back(vIndex[i] - 1);
				nIndices.push_back(nIndex[i] -1);
			}
		}
	}

	if (improveMesh) {
		normals = normals_tmp;

		std::vector<glm::vec3> extra_verts;
		std::vector<glm::vec3> extra_norms;
		std::map<std::pair<int, int>, int> mapping;

		normals.resize(vertices.size());
		std::vector<bool> tested(vertices.size(), false);

		for (int i = 0; i < indices.size(); ++i) {
			int vIdx = indices[i];
			int nIdx = nIndices[i];

			if (tested[vIdx] && vIdx != nIdx) {
				auto it = mapping.find(std::pair<int, int>(vIdx, nIdx));
				if (it != mapping.end())
					indices[i] = vertices.size() + it->second - 1;
				else {
					extra_verts.push_back(vertices[indices[i]]);
					extra_norms.push_back(normals_tmp[nIndices[i]]);
					indices[i] = vertices.size() + extra_verts.size() - 1;
					mapping[std::pair<int, int>(vIdx, nIdx)] = extra_verts.size();
				}
			}
			else {
				normals[indices[i]] = normals_tmp[nIndices[i]];
				tested[indices[i]] = true;
			}
		}

		for (auto it = tested.begin(); it != tested.end(); ++it)
			if (*it != true)
				cout << "probs" << endl;


		vertices.insert(vertices.end(), extra_verts.begin(), extra_verts.end());
		normals.insert(normals.end(), extra_norms.begin(), extra_norms.end());

		mapping.clear();
		extra_verts.clear();
		extra_norms.clear();
		nIndices.clear();
		normals_tmp.clear();


		if (vertices.size() != normals.size())
		{
			cerr << "Object data (vertices/normals) incensitent." << endl;
			std::cout << "Vertices " << vertices.size() << std::endl;
			std::cout << "Normals " << normals.size() << std::endl;
			exit(0);
		}
		else
		{
			cout << "model loaded: " << filename << endl;
		}
	}

}

simpleModel::~simpleModel() {
	glDeleteBuffers(3, vbo);
	indices.clear();
	vertices.clear();
	normals.clear();
}

void simpleModel::resizeToUniformCube()
{

		float xMin = vertices[0].x;
		float xMax = vertices[0].x;
		float yMin = vertices[0].y;
		float yMax = vertices[0].y;
		float zMin = vertices[0].z;
		float zMax = vertices[0].z;
	
		//Change into positive area
		for (int i = 0; i < this->vertices.size(); i++) {
			//x-Axis
			if (vertices[i].x < xMin) {
				xMin = vertices[i].x;
			}
			if (vertices[i].x > xMax) {
				xMax = vertices[i].x;
			}
	
			//y-Axis
			if (vertices[i].y < yMin) {
				yMin = vertices[i].y;
			}
			if (vertices[i].y > yMax) {
				yMax = vertices[i].y;
			}
	
			//z-Axis
			if (vertices[i].z < zMin) {
				zMin = vertices[i].z;
			}
			if (vertices[i].z > zMax) {
				zMax = vertices[i].z;
			}
		}
		
		glm::vec3 minDim = glm::vec3(xMin, yMin, zMin);
		glm::vec3 maxDim = glm::vec3(xMax, yMax, zMax);
		maxDim -= minDim;

		float maxCoordinate = glm::max(maxDim.x, glm::max(maxDim.y, maxDim.z));

		for (int i = 0; i < this->vertices.size(); i++) {
			vertices[i] -= minDim;
			vertices[i] /= maxCoordinate;
		}
}

void simpleModel::upload() {
	glGenBuffers(3, vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float)*3, vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, normals.size()*sizeof(float)*3, normals.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
}

void simpleModel::draw() {
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

void simpleModel::uploadPoints() {
	indices.clear();

	for (int i = 0; i < vertices.size(); i++) {
		indices.push_back(float(i));
	}

	glGenBuffers(3, vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) * 3, vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float) * 3, normals.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
}

void simpleModel::drawPoints() {
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
	glDrawElements(GL_POINTS, vertices.size(), GL_UNSIGNED_INT, 0);
}

void simpleModel::calculateNormals()
{
	//std::cout << vertices.size() << std::endl;
	//if (!normals.empty()) {
	//	std::cout << "skipping normal calculation" << std::endl;
	//	return;
	//}
	//else
	{
		std::cout << "calculating normals" << std::endl;
		normals.clear();
	}

	for (int i = 0; i < vertices.size(); i++) {
		normals.push_back(glm::vec3(0.0f));
	}

	for (int i = 0; i < indices.size(); i = i + 3) {
		glm::vec3 normal = glm::cross(vertices[indices[i + 1]] - vertices[indices[i]] , vertices[indices[i + 2]] - vertices[indices[i]]);
		normals[indices[i]] += normal;
		normals[indices[i + 1]] += normal;
		normals[indices[i + 2]] += normal;
	}

	for (int i = 0; i < normals.size(); i++) {
		normals[i] = glm::normalize(normals[i]);
	}
	
}


groundPlane::groundPlane(const float height, const float width) {
	vertices = vector<glm::vec3>(4);
	vertices = { glm::vec3(-width, height, -width),
		         glm::vec3(-width, height,  width),
				 glm::vec3( width, height,  width),
				 glm::vec3( width, height, -width) };

	normals = vector<glm::vec3>(4, glm::vec3(0.f, 1.f, 0.f));

	indices = vector<unsigned int>(6);
	indices = { 0, 1, 3, 1, 2, 3 };
}

solidTorus::solidTorus(const float r, const float R, const float sides, const float rings) {
	int i, j;
	float theta, phi, theta1;
	float cosTheta, sinTheta;
	float cosTheta1, sinTheta1;
	float ringDelta, sideDelta;


	ringDelta = 2.0 * M_PI / rings;
	sideDelta = 2.0 * M_PI / sides;

	theta = 0.0;
	cosTheta = 1.0;
	sinTheta = 0.0;


	for (i = rings - 1; i >=0; i--) {
		theta1 = theta + ringDelta;
		cosTheta1 = cos(theta1);
		sinTheta1 = sin(theta1);
		phi = 0.0;
		for (j = sides; j >= 0; j--) {
			float cosPhi, sinPhi, dist;

			phi += sideDelta;
			cosPhi = cos(phi);
			sinPhi = sin(phi);
			dist = R + r * cosPhi;

			normals.push_back(glm::vec3(cosTheta1 * cosPhi, -sinTheta1 * cosPhi, sinPhi));
			vertices.push_back(glm::vec3(cosTheta1 * dist, -sinTheta1 * dist, r * sinPhi));

			normals.push_back(glm::vec3(cosTheta * cosPhi, -sinTheta * cosPhi, sinPhi));
			vertices.push_back(glm::vec3(cosTheta * dist, -sinTheta * dist,  r * sinPhi));

			if (j < sides) {
				indices.push_back(vertices.size()-1 -2);
				indices.push_back(vertices.size()-1);
				indices.push_back(vertices.size()-1 -1);

				indices.push_back(vertices.size()-1 -2);
				indices.push_back(vertices.size()-1 -1);
				indices.push_back(vertices.size()-1 -3);
			}
		}

		theta = theta1;
		cosTheta = cosTheta1;
		sinTheta = sinTheta1;

	}
}
	

solidSphere::solidSphere(const float r, const int slices, const int stacks) {
	
	const float dTheta = 2.0*M_PI/(float)stacks;
	const float dPhi = M_PI/(float)slices;  
	
	for (int i = stacks; i>=0; i--) {
		glm::vec2 t(1-i*dTheta/(2.0*M_PI),1.0f);
    	glm::vec3 p(0,r,0);  
		vertices.push_back(p);
		uvs.push_back(t);
    } 

	const int tmpSize = vertices.size();
   
	//North pole
	for (int i = stacks; i>=0; i--) {
		glm::vec2 t(1-i*dTheta/(2.0*M_PI),(M_PI-dPhi)/M_PI);
		glm::vec3 p(r*sin(dPhi)*cos(i*dTheta), r*cos(dPhi), r*sin(dPhi)*sin(i*dTheta));		
		vertices.push_back(p);
		uvs.push_back(t);
    } 

	int triangleID = 0;
	for ( ;triangleID < stacks; triangleID++) 
	{
		indices.push_back(triangleID);
		indices.push_back(triangleID+tmpSize);
		indices.push_back(triangleID+tmpSize+1);
	}

	indices.push_back(stacks-1);
	indices.push_back(stacks*2 -1);
	indices.push_back(stacks);
	
	
	// Middle Part 
	 
	//	v0--- v2
	//  |  	/ |
	//  |  /  | 
	//  | /   |
	//  v1--- v3

	std::cout << "IMPORTANT: helper.cpp, solidSphere has been changed (not useable for triangle meshes anymore)!" << std::endl;

	for (int j=1; j < slices-1; j++) 
	{
		for (int i = stacks; i>0; i--)  //POINTCLOUD: Remove this = to stop the double point glitch
		{    			
			glm::vec2 t = glm::vec2 (1-i*dTheta/(2.0*M_PI),(M_PI-(j+1)*dPhi)/M_PI); 
			glm::vec3 p = glm::vec3 (r*sin((j+1)*dPhi)*cos(i*dTheta), r*cos((j+1)*dPhi), r*sin((j+1)*dPhi)*sin(i*dTheta));
			vertices.push_back(p);
			uvs.push_back(t);

			//add two triangles
 
			indices.push_back(vertices.size()  - stacks-2);	//v0
			indices.push_back(vertices.size() -1);			//v1
			indices.push_back(vertices.size()  - stacks-1);	//v2
 					 
			indices.push_back(vertices.size() - stacks-1);	//v2
			indices.push_back(vertices.size() - 1);			//v1
			indices.push_back(vertices.size() );			//v3
			 
		}
		
	}     

	const int lastVertex=vertices.size()-1;

	//South Pole
	for (int i = stacks; i>=0; i--) {
		glm::vec2 t(1-i*dTheta/(2.0*M_PI),0.0f);
		glm::vec3 p = glm::vec3 (0,-r,0);
		vertices.push_back(p);
		uvs.push_back(t);
    } 

	triangleID = 0;
	for ( ;triangleID < stacks; triangleID++) 
	{
		indices.push_back(lastVertex-stacks+triangleID);
		indices.push_back(lastVertex+triangleID+1);
		indices.push_back(lastVertex-stacks+triangleID+1);
	}

}

void solidSphere::upload() {
	glGenBuffers(3, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float)*3, vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, uvs.size()*sizeof(float)*2, uvs.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
}

void solidSphere::draw() {
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

// Image / Texture
Image::Image(const char* filename) {
	//png::image<png::rgb_pixel> image(filename);
	//height = image.get_height();
	//width = image.get_width();
	//pixels = new unsigned char[width*height*3];
	//for (size_t y = 0; y < height; ++y)
	//	for (size_t x = 0; x < width; ++x) {
	//		png::rgb_pixel px = image.get_pixel(x, height-y-1);
	//		const int i = 3*(y*width+x);
	//		pixels[i+0] = px.red;
	//		pixels[i+1] = px.green;
	//		pixels[i+2] = px.blue;
	//	}
	//load and decode
	// Load file and decode image.
	unsigned error = lodepng::decode(image, width, height, filename);

	// If there's an error, display it.
	if (error != 0)
	{
		std::cout << "error " << error << ": " << lodepng_error_text(error) << std::endl;
	}

}

Image::~Image() {
	//delete[] pixels;
	//pixels = NULL;
}

int Image::makeTexture() {
	GLuint id[1];
	glGenTextures(1, id);
	int index = id[0];
	glBindTexture(GL_TEXTURE_2D, index);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA8, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);
	return index;
}

/// Camera stuff
cameraSystem::cameraSystem(float delta, float mouseDelta, glm::vec3 pos) : delta(delta), mouseDelta(mouseDelta) {
	position = glm::vec4(pos, 1.0f);
	viewDir = glm::normalize(glm::vec4(-0.92f, -0.36, -0.01, 1.0f));
	upDir    = glm::vec4(0,1, 0,0);
	rightDir = glm::vec4(glm::normalize(glm::cross(glm::vec3(viewDir), glm::vec3(upDir))), 0.f);
}

void cameraSystem::moveForward(float delta) {
	position = position + (delta*viewDir);
}

void cameraSystem::moveBackward(float delta) {
	position = position - (delta*viewDir);
}

void cameraSystem::moveUp(float delta) {
	position = position + (delta*upDir);
}

void cameraSystem::moveDown(float delta) {
	position = position - (delta*upDir);
}

void cameraSystem::moveRight(float delta) {
	position = position + (delta*rightDir);
}

void cameraSystem::moveLeft(float delta) {
	position = position - (delta*rightDir);
}

void cameraSystem::yaw(float angle) {
	glm::mat4 R = glm::rotate(angle, glm::vec3(upDir)); 
	viewDir = R*viewDir;
	rightDir = R*rightDir;
}

void cameraSystem::pitch(float angle) {
	glm::mat4 R = glm::rotate(angle, glm::vec3(rightDir)); 
	viewDir = R*viewDir;
	//upDir = R*upDir;
}

void cameraSystem::roll(float angle) {
	glm::mat4 R = glm::rotate(angle, glm::vec3(viewDir)); 
	rightDir = R*rightDir;
	//upDir = R*upDir;
}


void cameraSystem::updateCameraMatrix() {
	const glm::vec3 eye = glm::vec3(this->position);
	const glm::vec3 center = glm::vec3(this->position + cam.viewDir);
	const glm::vec3 up = glm::vec3(this->upDir);
	viewMatrix = glm::lookAt(eye, center, up);
}

void cameraSystem::updateRotationMatrix(glm::mat4 view) {
	cameraRotation = view;
	cameraRotation[3][0] = 0.0f;
	cameraRotation[3][1] = 0.0f;
	cameraRotation[3][2] = 0.0f;
}

void cameraSystem::updateViewFrustrum() {
	glm::vec3 p = glm::vec3(this->position);
	glm::vec3 d = glm::vec3(glm::normalize(this->viewDir));
	glm::vec3 up = glm::vec3(this->upDir);
	glm::vec3 right = glm::vec3(this->rightDir);

	fc = p + d * farDist;
	ftl = fc + (up * Hfar / 2.0f) - (right * Wfar / 2.0f);
	ftr = fc + (up * Hfar / 2.0f) + (right * Wfar / 2.0f);
	fbl = fc - (up * Hfar / 2.0f) - (right * Wfar / 2.0f);
	fbr = fc - (up * Hfar / 2.0f) + (right * Wfar / 2.0f);

	nc = p + d * nearDist;

	ntl = nc + (up * Hnear / 2.0f) - (right * Wnear / 2.0f);
	ntr = nc + (up * Hnear / 2.0f) + (right * Wnear / 2.0f);
	nbl = nc - (up * Hnear / 2.0f) - (right * Wnear / 2.0f);
	nbr = nc - (up * Hnear / 2.0f) + (right * Wnear / 2.0f);
}


void cameraSystem::Update(){

	updateCameraMatrix();
	updateRotationMatrix(viewMatrix); //just do it right here

	glutPostRedisplay();
}

coordinateSystem::coordinateSystem()
{
	this->vertices = { glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(10.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 10.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 10.0f)

	};

	this->indices = { 0, 1, 2, 3, 4, 5 };

	this->color = { glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, 1.0f) };
}

coordinateSystem::~coordinateSystem()
{
	glDeleteBuffers(3, vbo);
	indices.clear();
	vertices.clear();
	color.clear();
}

void coordinateSystem::upload()
{
	glGenBuffers(3, this->vbo);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(float) * 3, this->vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, this->color.size() * sizeof(float) * 3, this->color.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbo[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(unsigned int), this->indices.data(), GL_STATIC_DRAW);
}


void coordinateSystem::draw()
{
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[1]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbo[2]);
	glDrawElements(GL_LINES, this->indices.size(), GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

viewFrustrum::viewFrustrum(glm::mat4& modelMatrix, glm::mat4& viewMatrix, glm::mat4& projMatrix, int frustrumToBoxes, glm::vec3& viewDirection)
{
	this->numBoxes = frustrumToBoxes;
	this->change(modelMatrix, viewMatrix, projMatrix);
}

void viewFrustrum::change(glm::mat4 & modelMatrix, glm::mat4 & viewMatrix, glm::mat4 & projMatrix)
{
	std::vector<glm::vec4> tempVertices = { glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f),	//Near
		glm::vec4(1.0f, -1.0f, -1.0f, 1.0f),										//Near
		glm::vec4(1.0f, 1.0f, -1.0f, 1.0f),											//Near
		glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f),										//Near
		glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f),										//Far
		glm::vec4(1.0f, -1.0f, 1.0f, 1.0f),											//Far
		glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),											//Far
		glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f) };										//Far

	this->vertices.resize(8);
	glm::mat4 temp = glm::inverse(projMatrix * viewMatrix * modelMatrix);

	for (int i = 0; i < 8; i++) {
		tempVertices[i] = temp * tempVertices[i];
		this->vertices[i] = glm::vec3(tempVertices[i]) / tempVertices[i].w;
	}

	this->indices = { 0, 1,
		1, 2,
		2, 3,
		3, 0,
		4, 5,
		5, 6,
		6, 7,
		7, 4,
		0, 4,
		1, 5,
		2, 6,
		3, 7 };

	this->color = { glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f),

		glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), };
}

void viewFrustrum::getPlaneNormal(bool drawCrossPlanes)
{
	//Do not use this after transforming into a boxes!!!
	//We have 8 vertices, 8 colors, 24 indices

	//Get the middle Point of each plane (just for visualization)
	this->vertices.push_back( ( this->vertices[0] + this->vertices[1] + this->vertices[2] + this->vertices[3] ) / 4.0f); //Vertex-Index: 8, near
	this->vertices.push_back((this->vertices[4] + this->vertices[5] + this->vertices[6] + this->vertices[7]) / 4.0f); //Vertex-Index: 9, far
	this->vertices.push_back((this->vertices[0] + this->vertices[3] + this->vertices[4] + this->vertices[7]) / 4.0f); //Vertex-Index: 10, left
	this->vertices.push_back((this->vertices[1] + this->vertices[2] + this->vertices[5] + this->vertices[6]) / 4.0f); //Vertex-Index: 11, right
	this->vertices.push_back((this->vertices[3] + this->vertices[2] + this->vertices[7] + this->vertices[6]) / 4.0f); //Vertex-Index: 12, up
	this->vertices.push_back((this->vertices[0] + this->vertices[1] + this->vertices[4] + this->vertices[5]) / 4.0f); //Vertex-Index: 13, down

	this->nearNormal = -glm::normalize(glm::cross(this->vertices[1] - this->vertices[0], this->vertices[3] - this->vertices[0]));
	this->nearPoint = this->vertices[1];

	this->farNormal = -glm::normalize(glm::cross(this->vertices[7] - this->vertices[4], this->vertices[5] - this->vertices[4]));
	this->farPoint = this->vertices[7];

	this->leftNormal = glm::normalize(glm::cross(this->vertices[0] - this->vertices[3], this->vertices[7] - this->vertices[3]));
	this->leftPoint = this->vertices[0];

	this->rightNormal = -glm::normalize(glm::cross(this->vertices[1] - this->vertices[2], this->vertices[5] - this->vertices[2]));
	this->rightPoint = this->vertices[1];

	this->upNormal = glm::normalize(glm::cross(this->vertices[7] - this->vertices[3], this->vertices[2] - this->vertices[3]));
	this->upPoint = this->vertices[7];

	this->downNormal = -glm::normalize(glm::cross(this->vertices[0] - this->vertices[1], this->vertices[5] - this->vertices[1]));
	this->downPoint = this->vertices[0];


	this->vertices.push_back(this->vertices[8] + 2.0f * this->nearNormal); //Vertex-Index: 14
	this->vertices.push_back(this->vertices[9] + 2.0f * this->farNormal); //Vertex-Index: 15
	this->vertices.push_back(this->vertices[10] + 2.0f * this->leftNormal); //Vertex-Index: 16
	this->vertices.push_back(this->vertices[11] + 2.0f * this->rightNormal); //Vertex-Index: 17
	this->vertices.push_back(this->vertices[12] + 2.0f * this->upNormal); //Vertex-Index: 18
	this->vertices.push_back(this->vertices[13] + 2.0f * this->downNormal); //Vertex-Index: 19

	//Give every new vertex the color pink
	for (int i = 0; i < 6; i++) {
		this->color.push_back(glm::vec3(1.0f,1.0f, 1.0f));
	}

	for (int i = 0; i < 6; i++) {
		this->color.push_back(glm::vec3(1.0f, 0.08f, 0.6f));
	}

	////Cross into the planes
	if (drawCrossPlanes) {
		int newIndices[60] = { 8, 14, //Normal
							9, 15, //Normal
							10, 16, //Normal
							11, 17, //Normal
							12, 18, //Normal
							13, 19, //Normal
							0, 8, 1, 8, 2, 8, 3, 8, //Cross inside plane (near)

							4, 9, 5, 9, 6, 9, 7, 9, //Cross inside plane

							0, 10, 3, 10, 4, 10, 7, 10, //Cross inside plane

							1, 11, 2, 11, 5, 11, 6, 11, //Cross inside plane

							3, 12, 2, 12, 7, 12, 6, 12, //Cross inside plane

							0, 13, 1, 13, 4, 13, 5, 13 //Cross inside plane
		};
		this->indices.insert(this->indices.end(), newIndices, newIndices + 60);
	}
	else {
		////Without cross
		int newIndices[12] = { 8, 14, //Normal
			9, 15, //Normal
			10, 16, //Normal
			11, 17, //Normal
			12, 18, //Normal
			13, 19, //Normal
		};
		this->indices.insert(this->indices.end(), newIndices, newIndices + 12);
	}
}

void viewFrustrum::frustrumToBoxes(glm::vec3& viewDirection)
{
	glm::vec3 normViewDir = glm::normalize(viewDirection);

	glm::vec3 dir0 = this->vertices[4] - this->vertices[0];
	//float lengthCamDir0 = glm::dot(dir0, normViewDir);
	float length0 = glm::length(dir0);
	dir0 = glm::normalize(dir0);
	float lengthCamDir0 = glm::dot((1.0f / float(numBoxes)) * length0 * dir0, normViewDir);

	glm::vec3 dir1 = this->vertices[5] - this->vertices[1];
	//float lengthCamDir1 = glm::dot(dir1, normViewDir);
	float length1 = glm::length(dir1);
	dir1 = glm::normalize(dir1);
	float lengthCamDir1 = glm::dot((1.0f / float(numBoxes)) * length1 * dir1, normViewDir);

	glm::vec3 dir2 = this->vertices[6] - this->vertices[2];
	//float lengthCamDir2 = glm::dot(dir2, normViewDir);
	float length2 = glm::length(dir2);
	dir2 = glm::normalize(dir2);
	float lengthCamDir2 = glm::dot((1.0f / float(numBoxes)) * length2 * dir2, normViewDir);

	glm::vec3 dir3 = this->vertices[7] - this->vertices[3];
	//float lengthCamDir3 = glm::dot(dir3, normViewDir);
	float length3 = glm::length(dir3);
	dir3 = glm::normalize(dir3);
	float lengthCamDir3 = glm::dot((1.0f / float(numBoxes)) * length3 * dir3, normViewDir);
	


	

	this->indices.clear();
	//this->indices.resize(this->numBoxes * 8 * 2);
	
	this->color.clear();

	std::vector<glm::vec3> corners(this->numBoxes * 8);
	
	for (int i = 0; i < this->numBoxes; i++) {
		float scale = float(i + 1) / float(numBoxes);
		corners[8 * i + 4] = this->vertices[0] + (scale) * length0 * dir0;
		corners[8 * i + 5] = this->vertices[1] + (scale) * length1 * dir1;
		corners[8 * i + 6] = this->vertices[2] + (scale) * length2 * dir2;
		corners[8 * i + 7] = this->vertices[3] + (scale) * length3 * dir3;
	 			 
		corners[8 * i + 0] = corners[8 * i + 4] + lengthCamDir0 * normViewDir;
		corners[8 * i + 1] = corners[8 * i + 5] + lengthCamDir1 * normViewDir;
		corners[8 * i + 2] = corners[8 * i + 6] + lengthCamDir2 * normViewDir;
		corners[8 * i + 3] = corners[8 * i + 7] + lengthCamDir3 * normViewDir;

		//Color
		this->color.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
		this->color.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
		this->color.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
		this->color.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
		this->color.push_back(HSVtoRGB(360.0f / float(i + 1), 1.0f, 1.0f));
		this->color.push_back(HSVtoRGB(360.0f / float(i + 1), 1.0f, 1.0f));
		this->color.push_back(HSVtoRGB(360.0f / float(i + 1), 1.0f, 1.0f));
		this->color.push_back(HSVtoRGB(360.0f / float(i + 1), 1.0f, 1.0f));


		//Bacck
		this->indices.push_back(8 * i + 4);
		this->indices.push_back(8 * i + 5);

		this->indices.push_back(8 * i + 5);
		this->indices.push_back(8 * i + 6);

		this->indices.push_back(8 * i + 6);
		this->indices.push_back(8 * i + 7);

		this->indices.push_back(8 * i + 7);
		this->indices.push_back(8 * i + 4);

		//Front
		this->indices.push_back(8 * i + 0);
		this->indices.push_back(8 * i + 1);

		this->indices.push_back(8 * i + 1);
		this->indices.push_back(8 * i + 2);

		this->indices.push_back(8 * i + 2);
		this->indices.push_back(8 * i + 3);

		this->indices.push_back(8 * i + 3);
		this->indices.push_back(8 * i + 0);

		//Sides
		this->indices.push_back(8 * i + 4);
		this->indices.push_back(8 * i + 0);

		this->indices.push_back(8 * i + 5);
		this->indices.push_back(8 * i + 1);

		this->indices.push_back(8 * i + 6);
		this->indices.push_back(8 * i + 2);

		this->indices.push_back(8 * i + 7);
		this->indices.push_back(8 * i + 3);
	}

	this->vertices = corners;
}

viewFrustrum::~viewFrustrum()
{
	glDeleteBuffers(3, this->vbo);
	glDeleteBuffers(3, this->vboQuad);
}

void viewFrustrum::upload()
{
	glGenBuffers(3, this->vbo);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(float) * 3, this->vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, this->color.size() * sizeof(float) * 3, this->color.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbo[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(unsigned int), this->indices.data(), GL_STATIC_DRAW);
}

void viewFrustrum::draw()
{
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo[1]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbo[2]);

	glDrawElements(GL_LINES, this->indices.size(), GL_UNSIGNED_INT, 0);
	//glDrawElements(GL_QUADS, this->indices.size(), GL_UNSIGNED_INT, 0);
}

void viewFrustrum::uploadQuad()
{
	//Indices for Quad
	this->indicesQuads = { 0, 1, 2, 3,
							4, 5, 6, 7,
							0, 3, 7, 4,
							1, 2, 6, 5,
							3, 2, 6, 7,
							0, 1, 5, 4
							};

	this->verticesQuads.resize(8);
	this->verticesQuads[0] = this->vertices[0] + 0.001f * (this->nearNormal + this->leftNormal + this->downNormal);
	this->verticesQuads[1] = this->vertices[1] + 0.001f * (this->nearNormal + this->rightNormal + this->downNormal);
	this->verticesQuads[2] = this->vertices[2] + 0.001f * (this->nearNormal + this->rightNormal + this->upNormal);
	this->verticesQuads[3] = this->vertices[3] + 0.001f * (this->nearNormal + this->leftNormal + this->upNormal);
												   
	this->verticesQuads[4] = this->vertices[4] + 0.001f * (this->farNormal + this->leftNormal + this->downNormal);
	this->verticesQuads[5] = this->vertices[5] + 0.001f * (this->farNormal + this->rightNormal + this->downNormal);
	this->verticesQuads[6] = this->vertices[6] + 0.001f * (this->farNormal + this->rightNormal + this->upNormal);
	this->verticesQuads[7] = this->vertices[7] + 0.001f * (this->farNormal + this->leftNormal + this->upNormal);

	this->colorQuads = { glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
							glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f),

							glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
							glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), };

	//Turn the draw line color to white
	for (int i = 0; i < 8; i++) {
		this->color[i] = glm::vec3(1.0f, 1.0f, 1.0f);
	}

	this->upload();

	glGenBuffers(3, this->vboQuad);

	glBindBuffer(GL_ARRAY_BUFFER, this->vboQuad[0]);
	glBufferData(GL_ARRAY_BUFFER, this->verticesQuads.size() * sizeof(float) * 3, this->verticesQuads.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->vboQuad[1]);
	glBufferData(GL_ARRAY_BUFFER, this->colorQuads.size() * sizeof(float) * 3, this->colorQuads.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboQuad[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indicesQuads.size() * sizeof(unsigned int), this->indicesQuads.data(), GL_STATIC_DRAW);
	
}

void viewFrustrum::drawQuad()
{
	this->draw();

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboQuad[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboQuad[1]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboQuad[2]);

	glDrawElements(GL_QUADS, this->indicesQuads.size(), GL_UNSIGNED_INT, 0);
}


