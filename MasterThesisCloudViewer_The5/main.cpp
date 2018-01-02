#define GLEW_STATIC //Using the static lib, so we need to enable it
#define _CRT_SECURE_NO_DEPRECATE
#include <iostream>
#include <GL/glew.h>
#include <GL/glut.h>
#include <Ant/AntTweakBar.h>
#include <memory>
#include <algorithm>
#include "helper.h"
#include "Shader.h"
#include "Skybox.h"
#include "times.h"
#include "InstancedMesh.h"
#include "FBO.h"
#include "Texture.h"
#include "glm/gtx/string_cast.hpp"
#include "marchingCubesVolume.h"

#include "HalfEdgeMesh.h"
#include "MeshResampler.h"
#include "nanoflannHelper.h"

#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

#include "PC_Octree.h"
#include "ObjToPcd.h"
#include "BinaryFileReader.h"
#include "PC_Viewer.h"

//#include <glm/gtc/matrix_transform.hpp>
//glm::mat4 test = glm::frustum

//Octree
PC_Octree* octree = nullptr;

//Time
Timer timer;
int frame;
long timeCounter, timebase;
char timeString[50];

//Resolution (has to be changed in helper.h too)
glm::vec2 resolution = glm::vec2(1024.0f, 768.0f);

//Externals
//cameraSystem cam(1.0f, 1.0f, glm::vec3(20.95f, 20.95f, -0.6f));
//cameraSystem cam(1.0f, 1.0f, glm::vec3(2.44f, 1.41f, 0.008f));
cameraSystem cam(1.0f, 1.0f, glm::vec3(6.36f, 2.94f, 0.05f));

glm::mat4 projMatrix;
glm::mat4 viewMatrix;

bool leftMouseClick;
int leftMouseClickX;
int leftMouseClickY;

//Skybox
Skybox skybox;
char* negz = "C:/Dev/Assets/SkyboxTextures/Yokohama2/negz.jpg";
char* posz = "C:/Dev/Assets/SkyboxTextures/Yokohama2/posz.jpg";
char* posy = "C:/Dev/Assets/SkyboxTextures/Yokohama2/posy.jpg";
char* negy = "C:/Dev/Assets/SkyboxTextures/Yokohama2/negy.jpg";
char* negx = "C:/Dev/Assets/SkyboxTextures/Yokohama2/negx.jpg";
char* posx = "C:/Dev/Assets/SkyboxTextures/Yokohama2/posx.jpg";

//Textures
FilterKernel* filter = 0;

//Shaders
Shader basicShader;
Shader modelLoaderShader;
Shader simpleSplatShader;
Shader basicColorShader;
Shader pointShader;
//FBO Shader
Shader quadScreenSizedShader;
Shader standardMiniColorFboShader;
Shader standardMiniDepthFboShader;
//FBO Shader
Shader pointGbufferShader;
Shader pointDeferredShader;
Shader pointFuzzyShader;
Shader pointFuzzyFinalShader;
Shader pointGbufferUpdatedShader;
Shader pointDeferredUpdatedShader;
Shader pointGbufferUpdated2ndPassShader;
//Pixel
Shader pixelShader;


//Filter
Shader gaussFilterShader;
Shader oneDimKernelShader;

//Skybox
Shader skyboxShader;

//Models
simpleModel *teaPot = 0;
solidSphere *sphere = 0;
simpleQuad * quad = 0;
coordinateSystem *coordSysstem = 0;
viewFrustrum * viewfrustrum = 0;
BinaryReadDraw *binaryDraw = 0;
PC_Viewer *viewer = nullptr;

//Frame buffer object
FBO *fbo = 0;
FBO *fbo2 = 0;

// tweak bar
TwBar *tweakBar;
bool wireFrameTeapot = false;
bool backfaceCull = false;
bool drawOctreeBox = false;
bool setViewFrustrum = false;
bool showFrustrumCull = false;
bool fillViewFrustrum = false;
bool debugView = true;
bool useGaussFilter = false;
int filterPasses = 5;
glm::vec3 lightPos = glm::vec3(10.0, 10.0, 0.0);
float glPointSizeFloat = 80.0f;
float depthEpsilonOffset = 0.0f;
typedef enum { QUAD_SPLATS, POINTS_GL } SPLAT_TYPE; SPLAT_TYPE m_currenSplatDraw = POINTS_GL;
typedef enum { SIMPLE, DEBUG, DEFERRED, TRIANGLE, KERNEL, DEFERRED_UPDATE, CULL_DEFERRED} RENDER_TYPE; RENDER_TYPE m_currenRender = CULL_DEFERRED;

/* *********************************************************************************************************
Helper Function
********************************************************************************************************* */
void drawFBO(FBO *_fbo) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glClearColor(0.2f, 0.2f, 0.2f, 1);
	//Color
	standardMiniColorFboShader.enable();
	_fbo->bindTexture(0);
	standardMiniColorFboShader.uniform("tex", 0);
	standardMiniColorFboShader.uniform("downLeft", glm::vec2(0.0f, 0.5f));
	standardMiniColorFboShader.uniform("upRight", glm::vec2(0.5f, 1.0f));
	quad->draw();
	_fbo->unbindTexture(0);
	standardMiniColorFboShader.disable();

	//Depth
	standardMiniDepthFboShader.enable();
	_fbo->bindDepth();
	standardMiniDepthFboShader.uniform("tex", 0);
	standardMiniDepthFboShader.uniform("downLeft", glm::vec2(0.5f, 0.5f));
	standardMiniDepthFboShader.uniform("upRight", glm::vec2(1.0f, 1.0f));
	quad->draw();
	_fbo->unbindDepth();
	standardMiniDepthFboShader.disable();

	//Normal
	standardMiniColorFboShader.enable();
	_fbo->bindTexture(1);
	standardMiniColorFboShader.uniform("tex", 0);
	standardMiniColorFboShader.uniform("downLeft", glm::vec2(0.0f, 0.0f));
	standardMiniColorFboShader.uniform("upRight", glm::vec2(0.5f, 0.5f));
	quad->draw();
	_fbo->unbindTexture(1);
	standardMiniColorFboShader.disable();

	//Position
	standardMiniColorFboShader.enable();
	_fbo->bindTexture(2);
	standardMiniColorFboShader.uniform("tex", 0);
	standardMiniColorFboShader.uniform("downLeft", glm::vec2(0.5f, 0.0f));
	standardMiniColorFboShader.uniform("upRight", glm::vec2(1.0f, 0.5f));
	quad->draw();
	_fbo->unbindTexture(2);
	standardMiniColorFboShader.disable();
}


/* *********************************************************************************************************
TweakBar
********************************************************************************************************* */
void setupTweakBar() {
	TwInit(TW_OPENGL_CORE, NULL);
	tweakBar = TwNewBar("Settings");

	TwAddSeparator(tweakBar, "Splat Draw", nullptr);
	TwEnumVal Splats[] = { { QUAD_SPLATS, "QUAD_SPLATS" },{ POINTS_GL, "POINTS_GL" } };
	TwType SplatsTwType = TwDefineEnum("MeshType", Splats, 2);
	TwAddVarRW(tweakBar, "Splats", SplatsTwType, &m_currenSplatDraw, NULL);
	TwAddVarRW(tweakBar, "glPointSize", TW_TYPE_FLOAT, &glPointSizeFloat, " label='glPointSize' min=0.0 step=10.0 max=1000.0");
	TwAddVarRW(tweakBar, "depthEpsilonOffset", TW_TYPE_FLOAT, &depthEpsilonOffset, " label='depthEpsilonOffset' min=-10.0 step=0.001 max=10.0");
	TwAddVarRW(tweakBar, "Light Position", TW_TYPE_DIR3F, &lightPos, "label='Light Position'");

	TwAddSeparator(tweakBar, "Utility", nullptr);
	TwAddSeparator(tweakBar, "Wireframe", nullptr);
	TwAddVarRW(tweakBar, "Wireframe Teapot", TW_TYPE_BOOLCPP, &wireFrameTeapot, " label='Wireframe Teapot' ");
	TwAddVarRW(tweakBar, "Backface Cull", TW_TYPE_BOOLCPP, &backfaceCull, " label='Backface Cull' ");

	TwAddSeparator(tweakBar, "Octree", nullptr);
	TwAddVarRW(tweakBar, "Octree Box", TW_TYPE_BOOLCPP, &drawOctreeBox, " label='Octree Box' ");

	TwAddSeparator(tweakBar, "Set Viewfrustrum", nullptr);
	TwAddVarRW(tweakBar, "ViewFrustrum", TW_TYPE_BOOLCPP, &setViewFrustrum, " label='ViewFrustrum' ");
	TwAddVarRW(tweakBar, "Fill Frustrum", TW_TYPE_BOOLCPP, &fillViewFrustrum, " label='Fill Frustrum' ");
	TwAddVarRW(tweakBar, "Frustrum Cull", TW_TYPE_BOOLCPP, &showFrustrumCull, " label='Frustrum Cull' ");

	TwAddSeparator(tweakBar, "Filter", nullptr);
	TwAddVarRW(tweakBar, "Use Gauss", TW_TYPE_BOOLCPP, &useGaussFilter, " label='Use Gauss' ");
	TwAddVarRW(tweakBar, "Passes", TW_TYPE_INT16, &filterPasses, " label='Passes' min=0 step=1 max=100");

	TwAddSeparator(tweakBar, "Debug Options", nullptr);
	TwEnumVal render[] = { {SIMPLE, "SIMPLE"}, {DEBUG, "DEBUG"}, {DEFERRED, "DEFERRED"}, { DEFERRED_UPDATE, "DEFERRED_UPDATE"}, {CULL_DEFERRED ,"CULL_DEFERRED"}, { TRIANGLE , "TRIANGLE"}, { KERNEL, "KERNEL"} };
	TwType renderTwType = TwDefineEnum("renderType", render, 7);
	TwAddVarRW(tweakBar, "render", renderTwType, &m_currenRender, NULL);
}

/* *********************************************************************************************************
Initiation
********************************************************************************************************* */

std::vector<glm::vec3> colorOctree = { glm::vec3(1.0f, 0.0f, 0.0f),
glm::vec3(1.0f, 0.6f, 0.0f),
glm::vec3(0.0f, 0.5f, 0.5f),
glm::vec3(1.0f, 1.0f, 1.0f),
glm::vec3(0.0f, 1.0f, 0.0f),
glm::vec3(0.0f, 0.0f, 0.0f),
glm::vec3(1.0f, 0.0f, 1.0f),
glm::vec3(0.0f, 0.0f, 1.0f),
};

std::vector<glm::mat4> modelMatrixOctree;

void init() {
	/*****************************************************************
	Error Messages :(
	*****************************************************************/
	//std::cout << "ERROR main: We normals seem to be missplaced. GL_FRONT should be GL_BACK!!!" << std::endl;

	/*****************************************************************
	VTK-File
	*****************************************************************/
	//load_VTKfile();
	//upload_VTKfile();

	/*****************************************************************
	Screen-Quad
	*****************************************************************/
	quad = new simpleQuad();
	quad->upload();

	/*****************************************************************
	Filter
	*****************************************************************/
	filter = new FilterKernel();


	/*****************************************************************
	obj-Models
	*****************************************************************/
	//teaPot = new simpleModel("C:/Dev/Assets/Teapot/teapot.obj", true);
	//std::cout << "TeaPot model: " << teaPot->vertices.size() << " vertices" << std::endl;

	//for (int i = 0; i < teaPot->vertices.size(); i++) {
	//	teaPot->vertices[i] = 2.0f * teaPot->vertices[i];
	//}
	//objToPcd(teaPot->vertices, teaPot->normals);


	/*****************************************************************
	obj-Models
	*****************************************************************/
	std::vector<glm::vec3> bigVertices, bigNormals, bigColors;
	std::vector<float> bigRadii;

	/*****************************************************************
	PointClouds
	*****************************************************************/
	viewer = new PC_Viewer("D:/Dev/Assets/Pointcloud/ATL_RGB_vehicle_scan-20171228T203225Z-001/ATL_RGB_vehicle_scan/Potree");

	binaryDraw = new BinaryReadDraw("D:/Dev/Assets/Pointcloud/ATL_RGB_vehicle_scan-20171228T203225Z-001/ATL_RGB_vehicle_scan/Potree/data/r/r024.bin");
	binaryDraw->upload();

	loadPcText(bigVertices, bigColors, "D:/Dev/Assets/Pointcloud/ATL_RGB_vehicle_scan-20171228T203225Z-001/ATL_RGB_vehicle_scan/Ply-Files/treeOnRoad_mini.txt");
	octree = new PC_Octree(bigVertices, bigColors, 100);

	/*************
	***CityFront
	**************/
	//loadBigFile(bigVertices, bigNormals, bigRadii, bigColors, "C:/Users/Kompie8/Documents/Visual Studio 2015/Projects/MasterThesis_The5/MasterThesis_The5/pointclouds/cityFrontRGB.big");
	//for (int i = 0; i < bigVertices.size(); i++) {
	//	bigVertices[i] -= bigVertices[bigVertices.size() - 1] - glm::vec3(0.0f, 15.0f, 0.0f);
	//	float halfPi = 1.5707f;
	//	bigVertices[i] = glm::mat3(1.0f, 0.0f, 0.0f, 0.0f, glm::cos(halfPi), -glm::sin(halfPi), 0.0f, glm::sin(halfPi), glm::cos(halfPi)) * bigVertices[i];
	//
	//	if (glm::dot(-bigVertices[i], bigNormals[i]) > 0) {
	//		bigNormals[i] = -bigNormals[i];
	//	}
	//
	//}
	//octree = new PC_Octree(bigVertices, bigNormals, bigRadii, 100);

	/*************
	***TeaPot
	**************/
	//loadBigFile(bigVertices, bigNormals, bigRadii, "C:/Users/The5/Documents/Visual Studio 2015/Projects/MasterThesis_The5/MasterThesis_The5/pointclouds/bigTeapotVNA_100.big");
	//octree = new PC_Octree(bigVertices, bigNormals, bigRadii, 10);

	/*************
	***NanoSuit
	**************/
	//loadPolyFile(bigVertices, bigNormals, bigRadii, bigColors, "C:/Dev/Assets/Nanosuit/nanosuit.ply");
	//octree = new PC_Octree(bigVertices, bigNormals, bigColors, bigRadii, 10);

	/*************
	***Sphere
	**************/
	//sphere = new solidSphere(1.0f, 30, 30);
	//std::vector<glm::vec3> sphereNormals;
	//std::vector<float> radiiSphere(sphere->vertices.size(), 1.0f);
	//for (int i = 0; i < sphere->vertices.size(); i++) {
	//	sphereNormals.push_back(sphere->vertices[i]);
	//	sphere->vertices[i] = 3.0f * sphere->vertices[i];
	//}
	//octree = new PC_Octree(sphere->vertices, sphereNormals, radiiSphere, 100);



	int counter = 0;
	for (int i = 0; i < 8; i++) {
		if (octree->root.bitMaskChildren[i] == 1) {
			modelMatrixOctree.push_back(glm::mat4(1.0f));
			octree->getAabbLeafUniforms(modelMatrixOctree[counter], octree->root.children[counter]);
			counter++;
		}
	}
	
	std::cout << "-> Main: modelMatrixOctree.size " << modelMatrixOctree.size() << std::endl;
	std::cout << "-> Main: octree->modelMatrixLowestLeaf.size() " << octree->modelMatrixLowestLeaf.size() << std::endl;

	/*****************************************************************
	Coordinate System
	*****************************************************************/
	coordSysstem = new coordinateSystem();
	coordSysstem->upload();

	/*****************************************************************
	View Frusturm
	*****************************************************************/
	viewfrustrum = new viewFrustrum(glm::mat4(1.0f), viewMatrix, projMatrix, 5, glm::vec3(cam.viewDir));
	viewfrustrum->upload();

	/*****************************************************************
	Skybox (Only for aesthetic reasons, can be deleted)
	*****************************************************************/
	skybox.createSkybox(negz, posz, posy, negy, negx, posx);

	/*****************************************************************
	FBO
	*****************************************************************/
	fbo = new FBO("Gbuffer", WIDTH, HEIGHT, FBO_GBUFFER_32BIT);
	gl_check_error("fbo");

	fbo2 = new FBO("Gbuffer", WIDTH, HEIGHT, FBO_GBUFFER_32BIT);
	gl_check_error("fbo2");

	/*****************************************************************
	Texture
	*****************************************************************/
}


void loadShader(bool init) {
	basicShader = Shader("./shader/basic.vs.glsl", "./shader/basic.fs.glsl");
	basicColorShader = Shader("./shader/basicColor.vs.glsl", "./shader/basicColor.fs.glsl");
	modelLoaderShader = Shader("./shader/modelLoader.vs.glsl", "./shader/modelLoader.fs.glsl");
	skyboxShader = Shader("./shader/skybox.vs.glsl", "./shader/skybox.fs.glsl");
	simpleSplatShader = Shader("./shader/simpleSplat.vs.glsl", "./shader/simpleSplat.fs.glsl", "./shader/simpleSplat.gs.glsl");
	pointShader = Shader("./shader/point.vs.glsl", "./shader/point.fs.glsl");

	//Deferred
	pointGbufferShader = Shader("./shader/PointGbuffer/pointGbuffer.vs.glsl", "./shader/PointGbuffer/pointGbuffer.fs.glsl");
	pointDeferredShader = Shader("./shader/PointGbuffer/pointDeferred.vs.glsl", "./shader/PointGbuffer/pointDeferred.fs.glsl");
	pointFuzzyShader = Shader("./shader/PointGbuffer/pointFuzzy.vs.glsl", "./shader/PointGbuffer/pointFuzzy.fs.glsl");
	pointFuzzyFinalShader = Shader("./shader/PointGbuffer/pointFuzzyFinal.vs.glsl", "./shader/PointGbuffer/pointFuzzyFinal.fs.glsl");
	
	//Updated
	pointGbufferUpdatedShader = Shader("./shader/PointGbuffer/pointGbufferUpdated.vs.glsl", "./shader/PointGbuffer/pointGbufferUpdated.fs.glsl");
	pointDeferredUpdatedShader = Shader("./shader/PointGbuffer/pointDeferredUpdated.vs.glsl", "./shader/PointGbuffer/pointDeferredUpdated.fs.glsl");
	pointGbufferUpdated2ndPassShader = Shader("./shader/PointGbuffer/pointGbufferUpdated2ndPass.vs.glsl", "./shader/PointGbuffer/pointGbufferUpdated2ndPass.fs.glsl");
	

	//FBO
	quadScreenSizedShader = Shader("./shader/FboShader/quadScreenSized.vs.glsl", "./shader/FboShader/quadScreenSized.fs.glsl");
	standardMiniColorFboShader = Shader("./shader/FboShader/standardMiniColorFBO.vs.glsl", "./shader/FboShader/standardMiniColorFBO.fs.glsl");
	standardMiniDepthFboShader = Shader("./shader/FboShader/standardMiniDepthFBO.vs.glsl", "./shader/FboShader/standardMiniDepthFBO.fs.glsl");
	gaussFilterShader = Shader("./shader/Filter/gaussFilter.vs.glsl", "./shader/Filter/gaussFilter.fs.glsl");

	//Pixel
	pixelShader = Shader("./shader/Pixel/pixel.vs.glsl", "./shader/Pixel/pixel.fs.glsl");

	//Debug-Shaders
	oneDimKernelShader = Shader("./shader/Filter/oneDimKernel.vs.glsl", "./shader/Filter/oneDimKernel.fs.glsl");
}

/* *********************************************************************************************************
Scenes: Unit cube + Pointcloud
********************************************************************************************************* */
//void standardScene() {
//	gl_check_error("standardScene");
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//	glEnable(GL_DEPTH_TEST);
//	glClearColor(0.3f, 0.3f, 0.3f, 1);
//}

void PixelScene() {
	//Clear
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

	glDisable(GL_CULL_FACE);

	/* ********************************************
	Coordinate System
	**********************************************/
	basicColorShader.enable();
	glm::mat4 modelMatrix = glm::scale(glm::vec3(1.0f));
	basicColorShader.uniform("modelMatrix", modelMatrix);
	basicColorShader.uniform("viewMatrix", viewMatrix);
	basicColorShader.uniform("projMatrix", projMatrix);
	coordSysstem->draw();
	basicColorShader.disable();


	/* ********************************************
	Pointcloud
	**********************************************/
	glEnable(GL_POINT_SPRITE);
	glEnable(GL_PROGRAM_POINT_SIZE);

	pixelShader.enable();


	pixelShader.uniform("viewMatrix", viewMatrix);
	pixelShader.uniform("projMatrix", projMatrix);

	pixelShader.uniform("glPointSize", glPointSizeFloat);

	//octree->drawPointCloud_PosCol();
	binaryDraw->draw();

	pixelShader.disable();

	glDisable(GL_POINT_SPRITE);
	glDisable(GL_PROGRAM_POINT_SIZE);

}

void standardScene() {
	//gl_check_error("standardScene");

	//Clear
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.3f, 0.3f, 0.3f, 1);

	/* ********************************************
	modelMatrix
	**********************************************/
	glm::mat4 modelMatrix = glm::scale(glm::vec3(1.0f));

	/* ********************************************
	Draw Skybox (Disable Culling, else we loose skybox!)
	**********************************************/
	glDisable(GL_CULL_FACE);

	//skyboxShader.enable();
	//skyboxShader.uniform("projMatrix", projMatrix);
	//skyboxShader.uniform("viewMatrix", cam.cameraRotation);
	//skybox.Draw(skyboxShader);
	//skyboxShader.disable();

	/* ********************************************
	Coordinate System
	**********************************************/
	basicColorShader.enable();
	modelMatrix = glm::scale(glm::vec3(1.0f));
	basicColorShader.uniform("modelMatrix", modelMatrix);
	basicColorShader.uniform("viewMatrix", viewMatrix);
	basicColorShader.uniform("projMatrix", projMatrix);
	coordSysstem->draw();
	basicColorShader.disable();

	/* ********************************************
	Octree
	**********************************************/
	if (drawOctreeBox) {
		basicShader.enable();


		basicShader.uniform("viewMatrix", viewMatrix);
		basicShader.uniform("projMatrix", projMatrix);

		//Draw Single Box
		//octree->getAabbUniforms(modelMatrix);
		//basicShader.uniform("modelMatrix", modelMatrix);
		//basicShader.uniform("col", glm::vec3(1.0f, 0.0f, 1.0f));
		//octree->drawBox();

		//Draw first level of boxes
		//for (int i = 0; i < modelMatrixOctree.size(); i++) {
		//	basicShader.uniform("modelMatrix", modelMatrixOctree[i]);
		//	basicShader.uniform("col", colorOctree[i]);
		//	octree->drawBox();
		//}

		//Draw finest level of boxes
		for (int i = 0; i < octree->modelMatrixLowestLeaf.size(); i++) {
			basicShader.uniform("modelMatrix", octree->modelMatrixLowestLeaf[i]);
			basicShader.uniform("col", octree->colorLowestLeaf[i]);
			octree->drawBox();
		}

		basicShader.disable();
	}

	/* ********************************************
	Simple Splat
	**********************************************/
	if (backfaceCull) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
	}
	else {
		glDisable(GL_CULL_FACE);
	}



	if (wireFrameTeapot) {
		glPolygonMode(GL_FRONT, GL_LINE);
		glPolygonMode(GL_BACK, GL_LINE);
	}

	switch (m_currenSplatDraw) {
	case(QUAD_SPLATS):
		simpleSplatShader.enable();
		modelMatrix = glm::scale(glm::vec3(1.0f));
		simpleSplatShader.uniform("modelMatrix", modelMatrix);
		simpleSplatShader.uniform("viewMatrix", viewMatrix);
		simpleSplatShader.uniform("projMatrix", projMatrix);
		simpleSplatShader.uniform("col", glm::vec3(1.0f, 0.0f, 0.0f));

		glActiveTexture(GL_TEXTURE0);
		filter->Bind();
		simpleSplatShader.uniform("filter_kernel", 0);

		octree->drawPointCloud();
		simpleSplatShader.disable();
		break;
	case(POINTS_GL):
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_POINT_SPRITE);
		glEnable(GL_PROGRAM_POINT_SIZE);

		pointShader.enable();
		modelMatrix = glm::scale(glm::vec3(1.0f));
		pointShader.uniform("modelMatrix", modelMatrix);
		pointShader.uniform("viewMatrix", viewMatrix);
		pointShader.uniform("projMatrix", projMatrix);
		pointShader.uniform("col", glm::vec3(0.0f, 1.0f, 0.0f));

		pointShader.uniform("nearPlane", 1.0f);
		pointShader.uniform("farPlane", 500.0f);
		pointShader.uniform("viewPoint", glm::vec3(cam.position));
		
		pointShader.uniform("glPointSize", glPointSizeFloat);
		pointShader.uniform("depthToPosTexture", false);

		octree->drawPointCloud();
		pointShader.disable();
		glDisable(GL_POINT_SPRITE);
		glDisable(GL_PROGRAM_POINT_SIZE);
	}
	if (wireFrameTeapot) {
		glPolygonMode(GL_FRONT, GL_FILL);
		glPolygonMode(GL_BACK, GL_FILL);
	}


	/* ********************************************
	View Frustrum
	**********************************************/
	if (setViewFrustrum) {
		setViewFrustrum = false;
		viewfrustrum->change(glm::mat4(1.0f), viewMatrix, projMatrix);
		//viewfrustrum->frustrumToBoxes(glm::vec3(cam.viewDir));
		viewfrustrum->getPlaneNormal(false);

		if (fillViewFrustrum) {
			viewfrustrum->uploadQuad();
		}
		else {
			viewfrustrum->upload();
		}
	}

	basicColorShader.enable();
	modelMatrix = glm::scale(glm::vec3(1.0f));
	basicColorShader.uniform("modelMatrix", modelMatrix);
	basicColorShader.uniform("viewMatrix", viewMatrix);
	basicColorShader.uniform("projMatrix", projMatrix);

	if (fillViewFrustrum) {
		viewfrustrum->drawQuad();
	}
	else {
		viewfrustrum->draw();
	}

	basicColorShader.disable();

	basicShader.enable();
	basicShader.uniform("viewMatrix", viewMatrix);
	basicShader.uniform("projMatrix", projMatrix);
	if (showFrustrumCull) {
		octree->initViewFrustrumCull(octree->root, *viewfrustrum);
		for (int i = 0; i < octree->modelMatrixLowestLeaf.size(); i++) {
			basicShader.uniform("modelMatrix", octree->modelMatrixLowestLeaf[i]);
			basicShader.uniform("col", octree->colorLowestLeaf[i]);
			octree->drawBox();
		}
	}
	basicShader.disable();




	///* ********************************************
	//VTK-Mesh
	//**********************************************/
	//basicShader.enable();
	//if (wireFrameTeapot) {
	//	glPolygonMode(GL_FRONT, GL_LINE);
	//	glPolygonMode(GL_BACK, GL_LINE);
	//}

	//glm::mat4 modelMatrix = glm::scale(glm::vec3(1.0f));

	//basicShader.uniform("modelMatrix", modelMatrix);
	//basicShader.uniform("viewMatrix", viewMatrix);
	//basicShader.uniform("projMatrix", projMatrix);

	//basicShader.uniform("col", glm::vec3(1.0f, 0.0f, 0.0f));

	////draw_VTKfile();
	////teaPot->draw();
	//teaPot->drawPoints();

	//if (wireFrameTeapot) {
	//	glPolygonMode(GL_FRONT, GL_FILL);
	//	glPolygonMode(GL_BACK, GL_FILL);
	//}

	//basicShader.disable();
}

void standardSceneFBO() {
	//gl_check_error("standardSceneFBO");
	/* #### FBO ####*/
	fbo->Bind();
	{
		//Clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.3f, 0.3f, 0.3f, 1);

		/* ********************************************
		modelMatrix
		**********************************************/
		glm::mat4 modelMatrix = glm::scale(glm::vec3(1.0f));

		/* ********************************************
		Draw Skybox (Disable Culling, else we loose skybox!)
		**********************************************/
		glDisable(GL_CULL_FACE);

		//skyboxShader.enable();
		//skyboxShader.uniform("projMatrix", projMatrix);
		//skyboxShader.uniform("viewMatrix", cam.cameraRotation);
		//skybox.Draw(skyboxShader);
		//skyboxShader.disable();

		/* ********************************************
		Coordinate System
		**********************************************/
		basicColorShader.enable();
		modelMatrix = glm::scale(glm::vec3(1.0f));
		basicColorShader.uniform("modelMatrix", modelMatrix);
		basicColorShader.uniform("viewMatrix", viewMatrix);
		basicColorShader.uniform("projMatrix", projMatrix);
		coordSysstem->draw();
		basicColorShader.disable();

		/* ********************************************
		Octree
		**********************************************/
		if (drawOctreeBox) {
			basicShader.enable();


			basicShader.uniform("viewMatrix", viewMatrix);
			basicShader.uniform("projMatrix", projMatrix);

			//Draw Single Box
			//octree->getAabbUniforms(modelMatrix);
			//basicShader.uniform("modelMatrix", modelMatrix);
			//basicShader.uniform("col", glm::vec3(1.0f, 0.0f, 1.0f));
			//octree->drawBox();

			//Draw first level of boxes
			//for (int i = 0; i < modelMatrixOctree.size(); i++) {
			//	basicShader.uniform("modelMatrix", modelMatrixOctree[i]);
			//	basicShader.uniform("col", colorOctree[i]);
			//	octree->drawBox();
			//}

			//Draw finest level of boxes
			for (int i = 0; i < octree->modelMatrixLowestLeaf.size(); i++) {
				basicShader.uniform("modelMatrix", octree->modelMatrixLowestLeaf[i]);
				basicShader.uniform("col", octree->colorLowestLeaf[i]);
				octree->drawBox();
			}

			basicShader.disable();
		}

		/* ********************************************
		Simple Splat
		**********************************************/
		if (backfaceCull) {
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
		}
		else {
			glDisable(GL_CULL_FACE);
		}



		if (wireFrameTeapot) {
			glPolygonMode(GL_FRONT, GL_LINE);
			glPolygonMode(GL_BACK, GL_LINE);
		}

		switch (m_currenSplatDraw) {
		case(QUAD_SPLATS):
			simpleSplatShader.enable();
			modelMatrix = glm::scale(glm::vec3(1.0f));
			simpleSplatShader.uniform("modelMatrix", modelMatrix);
			simpleSplatShader.uniform("viewMatrix", viewMatrix);
			simpleSplatShader.uniform("projMatrix", projMatrix);
			simpleSplatShader.uniform("col", glm::vec3(1.0f, 0.0f, 0.0f));
			octree->drawPointCloud();
			simpleSplatShader.disable();
			break;
		case(POINTS_GL):
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glEnable(GL_POINT_SPRITE);
			glEnable(GL_PROGRAM_POINT_SIZE);

			pointShader.enable();
			modelMatrix = glm::scale(glm::vec3(1.0f));
			pointShader.uniform("modelMatrix", modelMatrix);
			pointShader.uniform("viewMatrix", viewMatrix);
			pointShader.uniform("projMatrix", projMatrix);
			pointShader.uniform("col", glm::vec3(0.0f, 1.0f, 0.0f));

			pointShader.uniform("nearPlane", 1.0f);
			pointShader.uniform("farPlane", 500.0f);
			pointShader.uniform("viewPoint", glm::vec3(cam.position));
			pointShader.uniform("lightPos", lightPos);
			pointShader.uniform("glPointSize", glPointSizeFloat);

			pointShader.uniform("depthToPosTexture", true);

			octree->drawPointCloud();
			pointShader.disable();
			glDisable(GL_POINT_SPRITE);
			glDisable(GL_PROGRAM_POINT_SIZE);
			break;
		}
		if (wireFrameTeapot) {
			glPolygonMode(GL_FRONT, GL_FILL);
			glPolygonMode(GL_BACK, GL_FILL);
		}


		/* ********************************************
		View Frustrum
		**********************************************/
		if (setViewFrustrum) {
			setViewFrustrum = false;
			viewfrustrum->change(glm::mat4(1.0f), viewMatrix, projMatrix);
			//viewfrustrum->frustrumToBoxes(glm::vec3(cam.viewDir));
			viewfrustrum->getPlaneNormal(false);

			if (fillViewFrustrum) {
				viewfrustrum->uploadQuad();
			}
			else {
				viewfrustrum->upload();
			}
		}

		basicColorShader.enable();
		modelMatrix = glm::scale(glm::vec3(1.0f));
		basicColorShader.uniform("modelMatrix", modelMatrix);
		basicColorShader.uniform("viewMatrix", viewMatrix);
		basicColorShader.uniform("projMatrix", projMatrix);

		if (fillViewFrustrum) {
			viewfrustrum->drawQuad();
		}
		else {
			viewfrustrum->draw();
		}

		basicColorShader.disable();

		basicShader.enable();
		basicShader.uniform("viewMatrix", viewMatrix);
		basicShader.uniform("projMatrix", projMatrix);
		if (showFrustrumCull) {
			octree->initViewFrustrumCull(octree->root, *viewfrustrum);
			for (int i = 0; i < octree->modelMatrixLowestLeaf.size(); i++) {
				basicShader.uniform("modelMatrix", octree->modelMatrixLowestLeaf[i]);
				basicShader.uniform("col", octree->colorLowestLeaf[i]);
				octree->drawBox();
			}
		}
		basicShader.disable();
	}
	fbo->Unbind();


	/* #### FBO End #### */
	//GaussFilter
	if (useGaussFilter) {
		for (int i = 0; i < filterPasses; i++) {
			fbo2->Bind();
			{
				glClear(GL_COLOR_BUFFER_BIT);
				glDisable(GL_DEPTH_TEST);
				glClearColor(0.3f, 0.3f, 0.3f, 1);
				gaussFilterShader.enable();

				glActiveTexture(GL_TEXTURE0);
				fbo->bindTexture(0);
				gaussFilterShader.uniform("texColor", 0);
				glActiveTexture(GL_TEXTURE1);
				fbo->bindTexture(1, 1);
				gaussFilterShader.uniform("texNormal", 1);
				glActiveTexture(GL_TEXTURE2);
				fbo->bindTexture(2, 2);
				gaussFilterShader.uniform("texPosition", 2);
				glActiveTexture(GL_TEXTURE3);
				fbo->bindDepth(3);
				gaussFilterShader.uniform("texDepth", 3);

				gaussFilterShader.uniform("resolutionWIDTH", (float)resolution.x);
				gaussFilterShader.uniform("resolutionHEIGHT", (float)resolution.y);
				gaussFilterShader.uniform("radius", 1.0f);
				gaussFilterShader.uniform("dir", glm::vec2(1.0f, 0.0f));
				quad->draw();

				fbo->unbindTexture(0);
				fbo->unbindTexture(1);
				fbo->unbindTexture(2);
				fbo->unbindDepth();
				glActiveTexture(GL_TEXTURE0);

				gaussFilterShader.disable();
			}
			fbo2->Unbind();

			fbo->Bind();
			{
				glClear(GL_COLOR_BUFFER_BIT);
				glDisable(GL_DEPTH_TEST);
				glClearColor(0.3f, 0.3f, 0.3f, 1);
				gaussFilterShader.enable();

				glActiveTexture(GL_TEXTURE0);
				fbo2->bindTexture(0, 0);
				gaussFilterShader.uniform("texColor", 0);
				glActiveTexture(GL_TEXTURE1);
				fbo2->bindTexture(1, 1);
				gaussFilterShader.uniform("texNormal", 1);
				glActiveTexture(GL_TEXTURE2);
				fbo2->bindTexture(2, 2);
				gaussFilterShader.uniform("texPosition", 2);
				glActiveTexture(GL_TEXTURE3);
				fbo2->bindDepth(3);
				gaussFilterShader.uniform("texDepth", 3);

				gaussFilterShader.uniform("resolutionWIDTH", (float)resolution.x);
				gaussFilterShader.uniform("resolutionHEIGHT", (float)resolution.y);
				gaussFilterShader.uniform("radius", 1.0f);
				gaussFilterShader.uniform("dir", glm::vec2(0.0f, 1.0f));
				quad->draw();

				fbo2->unbindTexture(0);
				fbo2->unbindTexture(1);
				fbo2->unbindTexture(2);
				fbo2->unbindDepth();
				glActiveTexture(GL_TEXTURE0);
				gaussFilterShader.disable();
			}
			fbo->Unbind();
		}
	}

	//Render to screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glClearColor(0.2f, 0.2f, 0.2f, 1);
	//Color
	standardMiniColorFboShader.enable();
	fbo->bindTexture(0);
	standardMiniColorFboShader.uniform("tex", 0);
	standardMiniColorFboShader.uniform("downLeft", glm::vec2(0.0f, 0.5f));
	standardMiniColorFboShader.uniform("upRight", glm::vec2(0.5f, 1.0f));
	quad->draw();
	fbo->unbindTexture(0);
	standardMiniColorFboShader.disable();

	//Depth
	standardMiniDepthFboShader.enable();
	fbo->bindDepth();
	standardMiniDepthFboShader.uniform("tex", 0);
	standardMiniDepthFboShader.uniform("downLeft", glm::vec2(0.5f, 0.5f));
	standardMiniDepthFboShader.uniform("upRight", glm::vec2(1.0f, 1.0f));
	quad->draw();
	fbo->unbindDepth();
	standardMiniDepthFboShader.disable();

	//Normal
	standardMiniColorFboShader.enable();
	fbo->bindTexture(1);
	standardMiniColorFboShader.uniform("tex", 0);
	standardMiniColorFboShader.uniform("downLeft", glm::vec2(0.0f, 0.0f));
	standardMiniColorFboShader.uniform("upRight", glm::vec2(0.5f, 0.5f));
	quad->draw();
	fbo->unbindTexture(1);
	standardMiniColorFboShader.disable();

	//Position
	standardMiniColorFboShader.enable();
	fbo->bindTexture(2);
	standardMiniColorFboShader.uniform("tex", 0);
	standardMiniColorFboShader.uniform("downLeft", glm::vec2(0.5f, 0.0f));
	standardMiniColorFboShader.uniform("upRight", glm::vec2(1.0f, 0.5f));
	quad->draw();
	fbo->unbindTexture(2);
	standardMiniColorFboShader.disable();
}

void splattingGITscene() {
	fbo->Bind();{
		/* ********************************************
		modelMatrix
		**********************************************/
		glm::mat4 modelMatrix = glm::scale(glm::vec3(1.0f));

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		glEnable(GL_POINT_SPRITE);
		glEnable(GL_PROGRAM_POINT_SIZE);

		//glEnable(GL_BLEND);
		//glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		//glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE);
	   

		pointFuzzyShader.enable();

		pointFuzzyShader.uniform("modelview_matrix", viewMatrix * modelMatrix);
		pointFuzzyShader.uniform("projection_matrix", projMatrix);
		pointFuzzyShader.uniform("projection_matrix_inv", glm::inverse(projMatrix));

		//Kernel
		glActiveTexture(GL_TEXTURE0);
		filter->Bind();
		pointFuzzyShader.uniform("filter_kernel", 0);

		//Placeholder
		pointFuzzyShader.uniform("viewport", glm::vec4(0.0f, 0.0f, resolution.x, resolution.y));
		/*splat_renderer.cpp (449):
		Vector4f frustum_plane[6];

		Matrix4f const& projection_matrix = m_camera.get_projection_matrix();
		for (unsigned int i(0); i < 6; ++i)
		{
			frustum_plane[i] = projection_matrix.row(3) + (-1.0f + 2.0f
				* static_cast<float>(i % 2)) * projection_matrix.row(i / 2);
		}

		for (unsigned int i(0); i < 6; ++i)
		{
			frustum_plane[i] = (1.0f / frustum_plane[i].block<3, 1>(
				0, 0).norm()) * frustum_plane[i];
		}
		*/
		glm::vec4 frustum_plane[6];
		pointFuzzyShader.uniform("frustum_plane", frustum_plane);
		pointFuzzyShader.uniform("material_color", glm::vec3(1.0, 0.0, 0.0));
		pointFuzzyShader.uniform("material_shininess", 8.0f);
		pointFuzzyShader.uniform("radius_scale", 1.0f);
		pointFuzzyShader.uniform("ewa_radius", 1.0f);
		pointFuzzyShader.uniform("epsilon", 5.0f * 1e-3f);

		octree->drawPointCloud();

		pointFuzzyShader.disable();
		filter->Unbind();

		glDisable(GL_POINT_SPRITE);
		glDisable(GL_PROGRAM_POINT_SIZE);
	}fbo->Unbind();

	if (true) {
		drawFBO(fbo);
	}
	else {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		pointFuzzyFinalShader.enable();

		pointFuzzyFinalShader.uniform("projection_matrix_inv", glm::inverse(projMatrix));
		pointFuzzyFinalShader.uniform("viewport", glm::vec4(0.0f, 0.0f, resolution.x, resolution.y));
		pointFuzzyFinalShader.uniform("material_shininess", 8.0f);


		fbo->bindTexture(0, 0);
		pointFuzzyFinalShader.uniform("color_texture", 0);
		fbo->bindTexture(1, 1);
		pointFuzzyFinalShader.uniform("normal_texture", 1);
		fbo->bindDepth(2);
		pointFuzzyFinalShader.uniform("depth_texture", 2);

		quad->draw();
		pointFuzzyFinalShader.disable();
	}
}

void standardSceneDeferred() {
	/* ********************************************
	modelMatrix
	**********************************************/
	glm::mat4 modelMatrix = glm::scale(glm::vec3(1.0f));

	/* #### FBO ####*/
	fbo->Bind();
	{
		//Clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); //disable color rendering

		/* ********************************************
		Simple Splat
		**********************************************/
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_POINT_SPRITE);
		glEnable(GL_PROGRAM_POINT_SIZE);	

		pointGbufferShader.enable();
		modelMatrix = glm::scale(glm::vec3(1.0f));
		pointGbufferShader.uniform("modelMatrix", modelMatrix);
		pointGbufferShader.uniform("viewMatrix", viewMatrix);
		pointGbufferShader.uniform("projMatrix", projMatrix);
		pointGbufferShader.uniform("col", glm::vec3(0.0f, 1.0f, 0.0f));

		pointGbufferShader.uniform("depthEpsilonOffset", depthEpsilonOffset + 0.001f);

		pointGbufferShader.uniform("nearPlane", 1.0f);
		pointGbufferShader.uniform("farPlane", 500.0f);
		pointGbufferShader.uniform("viewPoint", glm::vec3(cam.position));
		pointGbufferShader.uniform("glPointSize", glPointSizeFloat);

		//std::cout << "cam pos: " << cam.position.x << " " << cam.position.y << " " << cam.position.z << " " << cam.position.w << std::endl;
		pointGbufferShader.uniform("cameraPos", glm::vec3(cam.position));

		octree->drawPointCloud();
		pointGbufferShader.disable();
		glDisable(GL_POINT_SPRITE);
		glDisable(GL_PROGRAM_POINT_SIZE);
			

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); //disable color rendering
	}
	fbo->Unbind();

	
	fbo->Bind();
	{
		glDepthMask(GL_FALSE);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_POINT_SPRITE);
		glEnable(GL_PROGRAM_POINT_SIZE);

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);

		pointGbufferShader.enable();
		modelMatrix = glm::mat4(1.0f);
		pointGbufferShader.uniform("modelMatrix", modelMatrix);
		pointGbufferShader.uniform("viewMatrix", viewMatrix);
		pointGbufferShader.uniform("projMatrix", projMatrix);
		pointGbufferShader.uniform("col", glm::vec3(0.0f, 1.0f, 0.0f));

		pointGbufferShader.uniform("depthEpsilonOffset", 0.0f);

		pointGbufferShader.uniform("nearPlane", 1.0f);
		pointGbufferShader.uniform("farPlane", 500.0f);
		pointGbufferShader.uniform("viewPoint", glm::vec3(cam.position));
		pointGbufferShader.uniform("glPointSize", glPointSizeFloat);

		octree->drawPointCloud();
		pointGbufferShader.disable();
		glDisable(GL_POINT_SPRITE);
		glDisable(GL_PROGRAM_POINT_SIZE);

		glDepthMask(GL_TRUE);

		glDisable(GL_BLEND);
	}
	fbo->Unbind();

	//drawFBO(fbo);

	//fbo2->Bind();
	//{
	//	glClear(GL_COLOR_BUFFER_BIT);
	//	glDisable(GL_DEPTH_TEST);
	//	glDisable(GL_BLEND);
	//	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	//	pointDeferredShader.enable();
	//	fbo->bindTexture(0, 0);
	//	pointDeferredShader.uniform("texColor", 0);
	//	fbo->bindTexture(1, 1);
	//	pointDeferredShader.uniform("texNormal", 1);
	//	fbo->bindTexture(2, 2);
	//	pointDeferredShader.uniform("texPosition", 2);
	//	fbo->bindDepth(3);
	//	pointDeferredShader.uniform("texDepth", 3);
	//	glActiveTexture(GL_TEXTURE4);
	//	filter->Bind();
	//	pointDeferredShader.uniform("filter_kernel", 4);
	//	glm::vec4 lightPosView = viewMatrix * glm::vec4(lightPos, 0.0);
	//	pointDeferredShader.uniform("lightVecV", glm::vec3(lightPosView));
	//	quad->draw();
	//	pointDeferredShader.disable();
	//}
	//fbo2->Unbind();
	//drawFBO(fbo2);

	/* #### FBO End #### */
	//GaussFilter
	if (false) {
		for (int i = 0; i < filterPasses; i++) {
			fbo2->Bind();
			{
				glClear(GL_COLOR_BUFFER_BIT);
				glDisable(GL_DEPTH_TEST);
				glClearColor(0.3f, 0.3f, 0.3f, 1);
				gaussFilterShader.enable();

				glActiveTexture(GL_TEXTURE0);
				fbo->bindTexture(0);
				gaussFilterShader.uniform("texColor", 0);
				glActiveTexture(GL_TEXTURE1);
				fbo->bindTexture(1, 1);
				gaussFilterShader.uniform("texNormal", 1);
				glActiveTexture(GL_TEXTURE2);
				fbo->bindTexture(2, 2);
				gaussFilterShader.uniform("texPosition", 2);
				glActiveTexture(GL_TEXTURE3);
				fbo->bindDepth(3);
				gaussFilterShader.uniform("texDepth", 3);

				gaussFilterShader.uniform("resolutionWIDTH", (float)resolution.x);
				gaussFilterShader.uniform("resolutionHEIGHT", (float)resolution.y);
				gaussFilterShader.uniform("radius", 1.0f);
				gaussFilterShader.uniform("dir", glm::vec2(1.0f, 0.0f));
				quad->draw();

				fbo->unbindTexture(0);
				fbo->unbindTexture(1);
				fbo->unbindTexture(2);
				fbo->unbindDepth();
				glActiveTexture(GL_TEXTURE0);

				gaussFilterShader.disable();
			}
			fbo2->Unbind();

			fbo->Bind();
			{
				glClear(GL_COLOR_BUFFER_BIT);
				glDisable(GL_DEPTH_TEST);
				glClearColor(0.3f, 0.3f, 0.3f, 1);
				gaussFilterShader.enable();

				glActiveTexture(GL_TEXTURE0);
				fbo2->bindTexture(0, 0);
				gaussFilterShader.uniform("texColor", 0);
				glActiveTexture(GL_TEXTURE1);
				fbo2->bindTexture(1, 1);
				gaussFilterShader.uniform("texNormal", 1);
				glActiveTexture(GL_TEXTURE2);
				fbo2->bindTexture(2, 2);
				gaussFilterShader.uniform("texPosition", 2);
				glActiveTexture(GL_TEXTURE3);
				fbo2->bindDepth(3);
				gaussFilterShader.uniform("texDepth", 3);

				gaussFilterShader.uniform("resolutionWIDTH", (float)resolution.x);
				gaussFilterShader.uniform("resolutionHEIGHT", (float)resolution.y);
				gaussFilterShader.uniform("radius", 1.0f);
				gaussFilterShader.uniform("dir", glm::vec2(0.0f, 1.0f));
				quad->draw();

				fbo2->unbindTexture(0);
				fbo2->unbindTexture(1);
				fbo2->unbindTexture(2);
				fbo2->unbindDepth();
				glActiveTexture(GL_TEXTURE0);
				gaussFilterShader.disable();
			}
			fbo->Unbind();
		}
	}

	//Deferred Shading (Use this to render directly to screen, else use the fbo to see debug)
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	pointDeferredShader.enable();
	glActiveTexture(GL_TEXTURE0);
	fbo->bindTexture(0, 0);
	pointDeferredShader.uniform("texColor", 0);
	glActiveTexture(GL_TEXTURE1);
	fbo->bindTexture(1, 1);
	pointDeferredShader.uniform("texNormal", 1);
	glActiveTexture(GL_TEXTURE2);
	fbo->bindTexture(2, 2);
	pointDeferredShader.uniform("texPosition", 2);
	glActiveTexture(GL_TEXTURE3);
	fbo->bindDepth(3);
	pointDeferredShader.uniform("texDepth", 3);

	glActiveTexture(GL_TEXTURE4);
	filter->Bind();
	pointDeferredShader.uniform("filter_kernel", 4);

	glm::vec4 lightPosView = viewMatrix * glm::vec4(lightPos, 0.0);
	pointDeferredShader.uniform("lightVecV", glm::vec3(lightPosView));
	quad->draw();
	pointDeferredShader.disable();

	//std::cout << "IMPORTANT: Because of the 2-pass-depthbuffer, we can only render if the distance epsilon is GREATER 0" << std::endl;
}

void standardSceneDeferredUpdate() {
	/* ********************************************
	modelMatrix
	**********************************************/
	glm::mat4 modelMatrix = glm::scale(glm::vec3(1.0f));
	glm::vec4 clearColor = glm::vec4(0.0, 0.0f, 0.0f, 0.0f);

	/* #### FBO ####*/
	fbo->Bind();
	{
		//Clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);

		//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		glEnable(GL_POINT_SPRITE);
		glEnable(GL_PROGRAM_POINT_SIZE);

		pointGbufferUpdatedShader.enable();
		modelMatrix = glm::scale(glm::vec3(1.0f));
		pointGbufferUpdatedShader.uniform("modelMatrix", modelMatrix);
		pointGbufferUpdatedShader.uniform("viewMatrix", viewMatrix);
		pointGbufferUpdatedShader.uniform("projMatrix", projMatrix);
		pointGbufferUpdatedShader.uniform("depthEpsilonOffset", depthEpsilonOffset + 0.001f);

		pointGbufferUpdatedShader.uniform("viewPoint", glm::vec3(cam.position));
		pointGbufferUpdatedShader.uniform("glPointSize", glPointSizeFloat);
		pointGbufferUpdatedShader.uniform("cameraPos", glm::vec3(cam.position));
		pointGbufferUpdatedShader.uniform("renderPass", 0);
		pointGbufferUpdatedShader.uniform("clearColor", clearColor);

		octree->drawPointCloud();
		pointGbufferUpdatedShader.disable();
		glDisable(GL_POINT_SPRITE);
		glDisable(GL_PROGRAM_POINT_SIZE);

		//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	}
	fbo->Unbind();

	//drawFBO(fbo);

	fbo2->Bind();
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_POINT_SPRITE);
		glEnable(GL_PROGRAM_POINT_SIZE);
		glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);

		
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		//glDisable(GL_BLEND);
		//glBlendFunc(GL_SOURCE0_ALPHA, GL_ONE);

		pointGbufferUpdated2ndPassShader.enable();
		modelMatrix = glm::mat4(1.0f);
		pointGbufferUpdated2ndPassShader.uniform("modelMatrix", modelMatrix);
		pointGbufferUpdated2ndPassShader.uniform("viewMatrix", viewMatrix);
		pointGbufferUpdated2ndPassShader.uniform("projMatrix", projMatrix);
		pointGbufferUpdated2ndPassShader.uniform("col", glm::vec3(0.0f, 1.0f, 0.0f));
		pointGbufferUpdated2ndPassShader.uniform("depthEpsilonOffset", depthEpsilonOffset + 0.001f);

		pointGbufferUpdated2ndPassShader.uniform("width", resolution.x);
		pointGbufferUpdated2ndPassShader.uniform("height", resolution.y);

		pointGbufferUpdated2ndPassShader.uniform("nearPlane", 1.0f);
		pointGbufferUpdated2ndPassShader.uniform("farPlane", 500.0f);
		pointGbufferUpdated2ndPassShader.uniform("viewPoint", glm::vec3(cam.position));
		pointGbufferUpdated2ndPassShader.uniform("glPointSize", glPointSizeFloat);
		pointGbufferUpdated2ndPassShader.uniform("renderPass", 1);
		pointGbufferUpdated2ndPassShader.uniform("clearColor", clearColor);

		fbo->bindTexture(0, 0);
		pointGbufferUpdated2ndPassShader.uniform("texDepth", 0);
		glActiveTexture(GL_TEXTURE1);
		filter->Bind();
		pointGbufferUpdated2ndPassShader.uniform("filter_kernel", 1);

		octree->drawPointCloud();
		pointGbufferUpdated2ndPassShader.disable();
		glDisable(GL_POINT_SPRITE);
		glDisable(GL_PROGRAM_POINT_SIZE);

		glDepthMask(GL_TRUE);

		glDisable(GL_BLEND);
	}
	fbo2->Unbind();

	//Deferred Shading (Use this to render directly to screen, else use the fbo to see debug)
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //Alpha Blending
	glDisable(GL_BLEND);

	glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
	pointDeferredUpdatedShader.enable();
	glActiveTexture(GL_TEXTURE0);
	fbo2->bindTexture(0, 0);
	pointDeferredUpdatedShader.uniform("texColor", 0);
	glActiveTexture(GL_TEXTURE1);
	fbo2->bindTexture(1, 1);
	pointDeferredUpdatedShader.uniform("texNormal", 1);
	glActiveTexture(GL_TEXTURE2);
	fbo2->bindTexture(2, 2);
	pointDeferredUpdatedShader.uniform("texPosition", 2);
	glActiveTexture(GL_TEXTURE3);
	fbo2->bindDepth(3);
	pointDeferredUpdatedShader.uniform("texDepth", 3);

	glActiveTexture(GL_TEXTURE4);
	filter->Bind();
	pointDeferredUpdatedShader.uniform("filter_kernel", 4);

	glm::vec4 lightPosView = viewMatrix * glm::vec4(lightPos, 0.0);
	pointDeferredUpdatedShader.uniform("lightVecV", glm::vec3(lightPosView));
	quad->draw();
	pointDeferredUpdatedShader.disable();
}

void standardSceneDeferredUpdateCull() {
	if (setViewFrustrum) {
	}
	else {
		viewfrustrum->change(glm::mat4(1.0f), viewMatrix, projMatrix);
		//viewfrustrum->frustrumToBoxes(glm::vec3(cam.viewDir));
		viewfrustrum->getPlaneNormal(false);
		viewfrustrum->upload();
	}

	/* ********************************************
	modelMatrix
	**********************************************/
	glm::mat4 modelMatrix = glm::scale(glm::vec3(1.0f));
	glm::vec4 clearColor = glm::vec4(0.0, 0.0f, 0.0f, 0.0f);

	/* #### FBO ####*/
	fbo->Bind();
	{
		//Clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);
		//glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);

		//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		glEnable(GL_POINT_SPRITE);
		glEnable(GL_PROGRAM_POINT_SIZE);

		pointGbufferUpdatedShader.enable();
		modelMatrix = glm::scale(glm::vec3(1.0f));
		pointGbufferUpdatedShader.uniform("modelMatrix", modelMatrix);
		pointGbufferUpdatedShader.uniform("viewMatrix", viewMatrix);
		pointGbufferUpdatedShader.uniform("projMatrix", projMatrix);
		pointGbufferUpdatedShader.uniform("depthEpsilonOffset", depthEpsilonOffset + 0.001f);

		pointGbufferUpdatedShader.uniform("viewPoint", glm::vec3(cam.position));
		pointGbufferUpdatedShader.uniform("glPointSize", glPointSizeFloat);
		pointGbufferUpdatedShader.uniform("cameraPos", glm::vec3(cam.position));
		pointGbufferUpdatedShader.uniform("renderPass", 0);
		pointGbufferUpdatedShader.uniform("clearColor", clearColor);

		octree->drawPointCloudInFrustrumPrep(octree->root, *viewfrustrum);

		pointGbufferUpdatedShader.disable();
		glDisable(GL_POINT_SPRITE);
		glDisable(GL_PROGRAM_POINT_SIZE);

		//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	}
	fbo->Unbind();

	//drawFBO(fbo);

	fbo2->Bind();
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_POINT_SPRITE);
		glEnable(GL_PROGRAM_POINT_SIZE);
		glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);


		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		//glDisable(GL_BLEND);
		//glBlendFunc(GL_SOURCE0_ALPHA, GL_ONE);

		pointGbufferUpdated2ndPassShader.enable();
		modelMatrix = glm::mat4(1.0f);
		pointGbufferUpdated2ndPassShader.uniform("modelMatrix", modelMatrix);
		pointGbufferUpdated2ndPassShader.uniform("viewMatrix", viewMatrix);
		pointGbufferUpdated2ndPassShader.uniform("projMatrix", projMatrix);
		pointGbufferUpdated2ndPassShader.uniform("col", glm::vec3(0.0f, 1.0f, 0.0f));
		pointGbufferUpdated2ndPassShader.uniform("depthEpsilonOffset", depthEpsilonOffset + 0.001f);

		pointGbufferUpdated2ndPassShader.uniform("width", resolution.x);
		pointGbufferUpdated2ndPassShader.uniform("height", resolution.y);

		pointGbufferUpdated2ndPassShader.uniform("nearPlane", 1.0f);
		pointGbufferUpdated2ndPassShader.uniform("farPlane", 500.0f);
		pointGbufferUpdated2ndPassShader.uniform("viewPoint", glm::vec3(cam.position));
		pointGbufferUpdated2ndPassShader.uniform("glPointSize", glPointSizeFloat);
		pointGbufferUpdated2ndPassShader.uniform("renderPass", 1);
		pointGbufferUpdated2ndPassShader.uniform("clearColor", clearColor);

		fbo->bindTexture(0, 0);
		pointGbufferUpdated2ndPassShader.uniform("texDepth", 0);
		glActiveTexture(GL_TEXTURE1);
		filter->Bind();
		pointGbufferUpdated2ndPassShader.uniform("filter_kernel", 1);

		octree->drawPointCloudInFrustrumPrep(octree->root, *viewfrustrum);
		pointGbufferUpdated2ndPassShader.disable();
		glDisable(GL_POINT_SPRITE);
		glDisable(GL_PROGRAM_POINT_SIZE);

		glDepthMask(GL_TRUE);

		glDisable(GL_BLEND);
	}
	fbo2->Unbind();

	//Deferred Shading (Use this to render directly to screen, else use the fbo to see debug)
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //Alpha Blending
	glDisable(GL_BLEND);

	glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
	pointDeferredUpdatedShader.enable();
	glActiveTexture(GL_TEXTURE0);
	fbo2->bindTexture(0, 0);
	pointDeferredUpdatedShader.uniform("texColor", 0);
	glActiveTexture(GL_TEXTURE1);
	fbo2->bindTexture(1, 1);
	pointDeferredUpdatedShader.uniform("texNormal", 1);
	glActiveTexture(GL_TEXTURE2);
	fbo2->bindTexture(2, 2);
	pointDeferredUpdatedShader.uniform("texPosition", 2);
	glActiveTexture(GL_TEXTURE3);
	fbo2->bindDepth(3);
	pointDeferredUpdatedShader.uniform("texDepth", 3);

	glActiveTexture(GL_TEXTURE4);
	filter->Bind();
	pointDeferredUpdatedShader.uniform("filter_kernel", 4);

	glm::vec4 lightPosView = viewMatrix * glm::vec4(lightPos, 0.0);
	pointDeferredUpdatedShader.uniform("lightVecV", glm::vec3(lightPosView));
	quad->draw();
	pointDeferredUpdatedShader.disable();
}

void kernelScene(){
	//Debug Render Filter
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glClearColor(0.0f, 0.0f, 0.0f, 1);
	oneDimKernelShader.enable();
	glActiveTexture(GL_TEXTURE0);
	filter->Bind();
	oneDimKernelShader.uniform("filter_kernel", 0);
	quad->draw();
	filter->Unbind();
	oneDimKernelShader.disable();
}

/* *********************************************************************************************************
Display + Main
********************************************************************************************************* */
void display() {
	//Timer
	timer.update();
	//FPS-Counter
	frame++;
	timeCounter = glutGet(GLUT_ELAPSED_TIME);
	if (timeCounter - timebase > 1000) {
		sprintf_s(timeString, "FPS:%4.2f", frame*1000.0 / (timeCounter - timebase));
		timebase = timeCounter;
		frame = 0;
		glutSetWindowTitle(timeString);
	}

	PixelScene();

	//switch (m_currenRender) {
	//case SIMPLE:
	//	standardScene();
	//	break;
	//case DEBUG:
	//	standardSceneFBO();
	//	break;
	//case DEFERRED:
	//	standardSceneDeferred();
	//	break;
	//case DEFERRED_UPDATE:
	//	standardSceneDeferredUpdate();
	//	break;
	//case CULL_DEFERRED:
	//	standardSceneDeferredUpdateCull();
	//	break;
	//case TRIANGLE:
	//	splattingGITscene();
	//	break;
	//case KERNEL:
	//	kernelScene();
	//	break;
	//};

	TwDraw(); //Draw Tweak-Bar

	glutSwapBuffers();
	glutPostRedisplay();

}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE | GLUT_STENCIL);

	glutCreateWindow("Basic Framework");

	setupTweakBar();

	GLenum err = glewInit();
	if (GLEW_OK != err) {
		std::cerr << "Error : " << glewGetErrorString(err) << std::endl;
	}

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMotionFunc(onMouseMove);
	glutMouseFunc(onMouseDown);
	glutReshapeFunc(reshape);
	glutIdleFunc(onIdle);

	glutSpecialFunc((GLUTspecialfun)TwEventSpecialGLUT);
	glutPassiveMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT);
	TwGLUTModifiersFunc(glutGetModifiers);

	initGL();

	init();

	glutMainLoop();

	TwTerminate();

	delete coordSysstem;
	delete octree;
	delete viewfrustrum;
	//delete_VTKfile();


	return 0;
}










