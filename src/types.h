#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

class Face;

class Vertex {
public:
	int id;
	glm::vec3 vertex;
	std::vector<Face*> faces;
	std::vector<Vertex*> edges;

	Vertex(int id, glm::vec3 vertex) : id(id), vertex(vertex) {};
	void addEdge(Vertex* v);
	void addFace(Face* f);
};


class Face {
public:
	int id;
	Vertex** vertices;
	glm::vec3 facepoint;

	Face(int id, Vertex** vertices) :id(id), vertices(vertices) {
		facepoint = glm::vec3(0, 0, 0);
		for (int i = 0; i < 4; i++) {
			facepoint += vertices[i]->vertex;
		}
		facepoint /= 4;

		for (int i = 0; i < 4; i++) {
			vertices[i]->addFace(this);
			vertices[i]->addEdge(vertices[(i + 1) % 4]);
			vertices[i]->addEdge(vertices[(i - 1) < 0? 3: i-1]);
		}
	};
};

struct Edge {
	int vids[2];
	glm::vec3 edgepoint;
	Vertex* e;
};

class Mesh {
public:
	std::vector<Vertex*> vertices;
	std::vector<Face*> faces;
	
	void addVertex(Vertex* vertex);
	void addFace(Face* face);
};