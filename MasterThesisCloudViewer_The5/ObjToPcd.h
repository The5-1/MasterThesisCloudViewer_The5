#pragma once
#include <iostream>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

void objToPcd(std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals) {
	if(vertices.size() != normals.size()){
		std::cout << "ERROR: Vertices size (" << vertices.size() << ") does not match normals size (" << normals.size() << ")" << std::endl;
	}

	//Create new file
	std::ofstream file;
	file.open("C:/Users/Kompie8/Documents/Visual Studio 2015/Projects/MasterThesis_The5/MasterThesis_The5/pointclouds/selfmadeTeapot.pcd");

	file << "# .PCD v.7 - Point Cloud Data file format\n";
	file << "VERSION .7\n";
	file << "FIELDS x y z normal_x normal_y normal_z\n";
	file << "SIZE 4 4 4 4 4 4\n";
	file << "TYPE F F F F F F\n";
	file << "COUNT 1 1 1 1 1 1\n";
	file << "WIDTH " << vertices.size() << "\n";
	file << "HEIGHT 1\n";
	file << "VIEWPOINT 0 0 0 1 0 0 0\n";
	file << "POINTS " << vertices.size() << "\n";
	file << "DATA ascii\n";
	for (int i = 0; i < vertices.size(); i++) {
		file <<"v " << vertices[i].x << " " << vertices[i].y << " " << vertices[i].z << " " << normals[i].x << " " << normals[i].y << " " << normals[i].z << "\n";
	}

	//Close file
	file.close();
}

void loadBigFile(std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<float>& radii, const char *filename) {
	vertices.clear();
	normals.clear();
	radii.clear();

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

		if (strcmp(lineHeader, "v") == 0)
		{
			glm::vec3 vertex;
			glm::vec3 normal;
			float radius;

			fscanf(file, "%f %f %f %f %f %f %f\n", &vertex.x, &vertex.y, &vertex.z, &normal.x, &normal.y, &normal.z, &radius);
			vertices.push_back(vertex);
			normals.push_back(normal);
			radii.push_back(radius);

			//std::cout << vertex.x << " " << vertex.y << " " << vertex.z << " " << normal.x << " " << normal.y << " " << normal.z << " " << radius << std::endl;
		}

		
	}

	fclose(file);
}

void loadBigFile(std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<float>& radii, std::vector<glm::vec3>& colors, const char *filename) {
	vertices.clear();
	normals.clear();
	radii.clear();

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

		if (strcmp(lineHeader, "v") == 0)
		{
			glm::vec3 vertex;
			glm::vec3 normal;
			float radius;
			glm::vec3 color;

			fscanf(file, "%f %f %f %f %f %f %f %f %f %f\n", &vertex.x, &vertex.y, &vertex.z, &normal.x, &normal.y, &normal.z, &radius, &color.x, &color.y, &color.z);
			vertices.push_back(vertex);
			normals.push_back(normal);
			radii.push_back(radius);
			color /= 255.0f;
			colors.push_back(color);
			//std::cout << vertex.x << " " << vertex.y << " " << vertex.z << " " << normal.x << " " << normal.y << " " << normal.z << " " << radius << std::endl;
		}


	}
	fclose(file);
}

void loadPolyFile(std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<float>& radii, std::vector<glm::vec3>& colors, const char *filename) {
	vertices.clear();
	normals.clear();
	radii.clear();
	colors.clear();

	std::cout << "Load poly file" << std::endl;

	FILE * file = fopen(filename, "r");
	if (file == NULL) {
		cerr << "Model file not found: " << filename << endl;
		exit(0);
	}

	int numVertices = 0;

	while (1)
	{
		char lineHeader[128];
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break;

		if (strcmp(lineHeader, "element") == 0)
		{		
			fscanf(file, "%s", lineHeader);
			if (strcmp(lineHeader, "vertex") == 0)
			{
				fscanf(file, "%i\n", &numVertices);
				//std::cout << numVertices << std::endl;
			}
		}

		if (strcmp(lineHeader, "end_header") == 0){
			for (int i = 0; i < numVertices; i++) {
				glm::vec3 vertex;
				glm::vec3 normal;
				glm::vec3 color;
				float alpha;

				fscanf(file, "%f %f %f %f %f %f %f %f %f %f\n", &vertex.x, &vertex.y, &vertex.z, &normal.x, &normal.y, &normal.z, &color.x, &color.y, &color.z, &alpha);
				vertices.push_back(vertex);
				normals.push_back(normal);
				radii.push_back(1.0f);
				color /= 255.0f;
				colors.push_back(color);
			}

			break;
		}

	}
	fclose(file);
}

void loadPcText(std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& colors, const char *filename) {
	vertices.clear();
	colors.clear();

	FILE * file = fopen(filename, "r");
	if (file == NULL) {
		cerr << "Model file not found: " << filename << endl;
		exit(0);
	}

	char lineHeader[128];
	fgets(lineHeader, 128, file); //Reads the first line, which contains the build of the file (We just assume they are always build the same way)

	int numVertices;
	fscanf(file, "%i\n", &numVertices);
	std::cout << "Load Point-Cloud from Text file: " << numVertices << " numVertices" << std::endl;

	for (int i = 0; i < numVertices; i++) {
		double X, Y, Z; 
		float Xf, Yf, Zf;
		glm::vec3 color;
		float Point_Source_ID, Scan_Angle_Rank, Time, Intensity, Classification;

		fscanf(file, "%lf %lf %lf %f %f %f %f %f %f %f %f\n", &X, &Y, &Z, &color.x, &color.y, &color.z, &Point_Source_ID, &Scan_Angle_Rank, &Time, &Intensity, &Classification);

		//For the street PC, sadly is has crazy points
		X -= 2240700.0;
		Y -= 1363431.0;
		Z -= 1000.0;

		Xf = float(X);
		Yf = float(Y);
		Zf = float(Z);

		vertices.push_back(glm::vec3(Xf, Yf, Zf));
		colors.push_back(color);

		//std::cout << Xf << " " << Yf << " " << Zf << " " << color.x << " " << color.y << " " << color.z << std::endl;
	}


	fclose(file);
}


//void readLasFile() {
//	std::ifstream ifs;
//	ifs.open("D:/Dev/Assets/Pointcloud/ATL_RGB_vehicle_scan - 20171228T203225Z - 001/ATL_RGB_vehicle_scan/Las - Files/ScanLook_Vehicle01.las", std::ios::in | std::ios::binary);
//
//	LASreadOpener lasreadopener;
//	lasreadopener.set_file_name("D:/Dev/Assets/Pointcloud/ATL_RGB_vehicle_scan - 20171228T203225Z - 001/ATL_RGB_vehicle_scan/Las - Files/ScanLook_Vehicle01.las");
//
//	if (!lasreadopener.active())
//	{
//		fprintf(stderr, "ERROR: no input specified\n");
//	}
//
//	LASreader* lasreader = lasreadopener.open();
//	if (lasreader == 0)
//	{
//		fprintf(stderr, "ERROR: could not open lasreader\n");
//	}
//
//	fprintf(stderr, "reading %I64d points from '%s'.\n", lasreader->npoints, lasreadopener.get_file_name());
//
//	lasreader->close();
//	delete lasreader;
//}