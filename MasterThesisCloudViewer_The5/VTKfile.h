#pragma once
#include <glm\glm.hpp>
#include <vector>
#include <cstring>
#include <utility>
#include <string>
#include <iostream>
#include <errno.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GL/glut.h>

using namespace std;

class VTKfile {
public:
	char* fileName = "mesh.vtk";

	std::vector<unsigned int> indices;
	std::vector<glm::vec3> vertices;

	GLuint vbo[2];

public:
	/*void load_VTKfile() {
		FILE *filepoint;
		errno_t err;

		err = fopen_s(&filepoint, fileName, "r");

		if (filepoint == NULL) {
			cerr << "Model file not found: " << fileName << endl;
			exit(0);
		}

		while (1)
		{
			char lineHeader[128];
			int res = fscanf_s(filepoint, "%s", lineHeader);
			if (res == EOF)
				break;

			if (strcmp(lineHeader, "POINTS") == 0)
			{
				int numPoints;
				fscanf_s(filepoint, "%d %s \n", &numPoints);
				for (int i = 0; i < numPoints; i++) {
					glm::vec3 vertex;
					fscanf_s(filepoint, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
					vertices.push_back(vertex);
					std::cout << vertex.x << " " << vertex.y << " " << vertex.z << std::endl;
				}
			}

			if (strcmp(lineHeader, "POLYGONS") == 0)
			{
				int numPolygons;
				int numPolygonsTimeFour;

				fscanf_s(filepoint, "%d %d \n", &numPolygons, &numPolygonsTimeFour);
				for (int i = 0; i < numPolygons; i++) {
					glm::vec3 vertex;
					int three, index0, index1, index2;
					fscanf_s(filepoint, "%d %d %d %d\n", &three, &index0, &index1, &index2);
					indices.push_back(index0);
					indices.push_back(index1);
					indices.push_back(index2);
					std::cout << index0 << " " << index1 << " " << index2 << std::endl;
				}
			}
		}
	}*/

	void load_VTKfile();

	void upload_VTKfile();

	void draw_VTKfile();

	void delete_VTKfile();


};