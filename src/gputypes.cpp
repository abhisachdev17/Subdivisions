#include "gputypes.h"
#include <vector>

int getEdgeIndex(GPUVertex v, int edge) {
	for (int i = 0; i < v.currentEdge; i++) {
		if (v.edges[i] == v.id) {
			return i;
		}
	}

	return -1;
}

void addEdge(GPUVertex v, int edge, GPUMesh *m) {
	if (getEdgeIndex(v, edge) == -1) {
		v.edges[v.currentEdge] = edge;
		v.currentEdge++;
		m->vertices[v.id - 1] = v;
	}
}

void addEdgesToVertex(GPUMesh *m , GPUFace f) {
	for (int i = 0; i < 4; i++) {
		GPUVertex v = m->vertices[f.vertices[i] - 1];
		int edge = m->vertices[f.vertices[(i + 1) % 4] - 1].id;
		addEdge(v, edge, m);
		edge = m->vertices[f.vertices[(i - 1) < 0 ? 3 : i - 1] - 1].id;
		addEdge(v, edge, m);
	}
}

void addFaceToVertex(GPUMesh *m, GPUFace f) {
	for (int i = 0; i < 4; i++) {
		GPUVertex v = m->vertices[f.vertices[i] - 1];
		v.faces[v.currentFace] = f.id;
		v.currentFace++;
		m->vertices[f.vertices[i] - 1] = v;
	}
}

void GPUMesh::addVertex(GPUVertex v) {
	vertices.push_back(v);
}

void GPUMesh::addFace(GPUFace f) {
	faces.push_back(f);
	addFaceToVertex(this, f);
	addEdgesToVertex(this, f);
}
