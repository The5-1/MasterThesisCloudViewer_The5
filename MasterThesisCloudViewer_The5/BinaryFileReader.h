#pragma once
#include <fstream>
#include <iterator>
#include <algorithm>
#include <iostream>

#include <glm\glm.hpp>
#include <GL/glew.h>
#include <GL/glut.h>

#include <bitset>
//Potree binary format documentation:
//https://github.com/PropellerAero/potree-propeller-private/blob/master/docs/file_format.md

void readHrcFile(const char *filename) {
	std::ifstream file_to_open(filename, std::ios::binary);
	unsigned long int totalPoints = 0;

	if (file_to_open.is_open()) {
		while (!file_to_open.eof()) {


			unsigned long int numPoints;
			unsigned char bitMaskChar; //Unsigned char has a size of 1 byte ( = 8 bits)
			std::bitset<8> bitMask;

			file_to_open.read((char*)&bitMaskChar, sizeof(unsigned char));
			//Transfrom unsigned char to bits: https://stackoverflow.com/questions/37487528/how-to-get-the-value-of-every-bit-of-an-unsigned-char
			for (int i = 0; i != 8; i++) {
				bitMask[i] = (bitMaskChar & (1 << i)) != 0;
			}

			file_to_open.read((char*)&numPoints, sizeof(unsigned long int));
			totalPoints += numPoints;
			

			//std::cout << "Num Poitnts: " << numPoints << ", BitMask: (" << bitMask[0] << bitMask[1] << bitMask[2] << bitMask[3] << bitMask[4] << bitMask[5] << bitMask[6] << bitMask[7] << ")" << std::endl;
		}
	}

	//std::cout << "Total num points: " << totalPoints << std::endl;
}

void readBinaryFile(vector<glm::vec3> &vertices, vector<glm::vec3> &colors, const char *filename) {
	vertices.clear();
	colors.clear();

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

			vertices.push_back(glm::vec3(Xf, Yf, Zf));
			colors.push_back(glm::vec3(float(R)/255.0f, float(G) / 255.0f, float(B) / 255.0f));
			//std::cout << "("<<Xf << ","<< Yf << "," <<Zf <<") ("<< +R<<"," << +G <<"," << +B <<"," << +A << ")"  << std::endl;
		}
	}
}

class BinaryReadDraw {
public:
	GLuint vboPC[2];
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> colors;

public:
	BinaryReadDraw(const char *filename) {
		readBinaryFile(this->vertices, this->colors, filename);
		readHrcFile("D:/Dev/Assets/Pointcloud/ATL_RGB_vehicle_scan-20171228T203225Z-001/ATL_RGB_vehicle_scan/Potree/data/r/r.hrc");
	}

	void upload() {
		glGenBuffers(2, vboPC);

		glBindBuffer(GL_ARRAY_BUFFER, vboPC[0]);
		glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(float) * 3, this->vertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, vboPC[1]);
		glBufferData(GL_ARRAY_BUFFER, this->colors.size() * sizeof(float) * 3, this->colors.data(), GL_STATIC_DRAW);
	}

	void draw() {
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vboPC[0]);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, vboPC[1]);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glDrawArrays(GL_POINTS, 0, this->vertices.size());
	}

};