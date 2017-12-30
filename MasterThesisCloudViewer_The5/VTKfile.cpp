#define _CRT_SECURE_NO_DEPRECATE
#include "VTKfile.h"

void VTKfile::load_VTKfile() {
	FILE *filepoint;

	filepoint = fopen(fileName, "r");

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
}





void VTKfile::upload_VTKfile() {
	glGenBuffers(2, vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) * 3, vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
}

void VTKfile::draw_VTKfile() {
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

void VTKfile::delete_VTKfile() {
	glDeleteBuffers(2, vbo);
	indices.clear();
	vertices.clear();
}