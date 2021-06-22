#pragma once
#include<vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Vert {
	float x = 0.0, y = 0.0, z = 0.0;
};

struct GPUEdge {
	int id = 0;
	int face;
	int vertices[2];
};

struct GPUFace {
	int id = 0;
	int vertices[4]; // one face can only have upto 4 vertices
	Vert facepoint;
};

struct GPUVertex {
	int id = 0;
	int currentFace = 0;
	int currentEdge = 0;
	int faces[10]; // max 10 connected faces
	int edges[20]; // max 20 connected edges
	Vert vertex;
};


class GPUMesh {
public: 
	std::vector<GPUFace> faces;
	std::vector<GPUVertex> vertices;
	std::vector<GPUEdge> edges;

	void addVertex(GPUVertex v);
	void addFace(GPUFace f);
};
