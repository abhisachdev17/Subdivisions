#include "types.h"
#include <vector>

void Vertex::addEdge(Vertex* v) {
	auto it = std::find(edges.begin(), edges.end(), v);
	if (it == edges.end())
	{
		edges.push_back(v);
	}
}

void Vertex::addFace(Face* f) {
	faces.push_back(f);
}

void Mesh::addVertex(Vertex* v) {
	vertices.push_back(v);
}

void Mesh::addFace(Face* f) {
	faces.push_back(f);
}