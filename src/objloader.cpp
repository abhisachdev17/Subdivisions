#include "objloader.h"
#include <vector>
#include "types.h"
#include "gputypes.h"
#include <fstream>
#include <sstream>
#include <string>

void readFace(std::string line, Mesh * m, int id) {
	std::istringstream is(line);
	char f;
	int v1, v2, v3, v4;
	char slash;

	auto pos = line.find_first_of("/");
	if (pos == std::string::npos) {
		is >> f >> v1 >> v2 >> v3 >> v4;
	}
	else if (line.find_first_of("/", pos + 1) != pos + 1) {
		int n;
		is >> f >> v1 >> slash >> n >> v2 >> slash >> n >> v3 >> slash >> n >> v4;
	}
	else {
		int n;
		is >> f >> v1 >> slash >> slash >> n >> v2 >> slash >> slash >> n >> v3 >> slash >> slash >> n >> v4;
	}

	Vertex** vs = new Vertex * [4];
	vs[0] = m->vertices[v1 - 1];
	vs[1] = m->vertices[v2 - 1];
	vs[2] = m->vertices[v3 - 1];
	vs[3] = m->vertices[v4 - 1];

	Face* face = new Face(id, vs);
	m->addFace(face);
}

void readVertex(std::string line, Mesh * m, int id) {
	std::istringstream is(line);
	char v;
	float x, y, z;

	is >> v >> x >> y >> z;
	glm::vec3 pos = glm::vec3(x, y, z);

	Vertex* vertex = new Vertex(id, pos);
	m->addVertex(vertex);
}

void readFace(std::string line, GPUMesh* m, int id) {
	std::istringstream is(line);
	char f;
	int v1, v2, v3, v4;
	char slash;

	auto pos = line.find_first_of("/");
	if (pos == std::string::npos) {
		is >> f >> v1 >> v2 >> v3 >> v4;
	}
	else if (line.find_first_of("/", pos+1) != pos+1) {
		int n;
		is >> f >> v1 >> slash >> n >> v2 >> slash >> n >> v3 >> slash >> n >> v4;
	}
	else {
		int n;
		is >> f >> v1 >> slash >> slash >> n >> v2 >> slash >> slash >> n >> v3 >> slash >> slash >> n >> v4;
	}

	GPUFace face;
	face.id = id;
	face.vertices[0] = v1;
	face.vertices[1] = v2;
	face.vertices[2] = v3;
	face.vertices[3] = v4;

	m->addFace(face);
}

void readVertex(std::string line, GPUMesh* m, int id) {
	std::istringstream is(line);
	char v;
	float x, y, z;

	is >> v >> x >> y >> z;
	Vert pos;
	pos.x = x;
	pos.y = y;
	pos.z = z;

	GPUVertex vertex;
	vertex.id = id;
	vertex.vertex = pos;

	m->addVertex(vertex);
}

Mesh* loadObj(std::string path) {
	std::ifstream ifs (path);
	
	if (!ifs.is_open()) {
		return nullptr;
	}
	Mesh* mesh = new Mesh();
	std::string line;
	int vid = 1;
	int fid = 1;
	while (std::getline(ifs, line)) {

		if (line[0] == 'v' && (line[1] != 'n' && line[1] != 't')) {
			readVertex(line, mesh, vid);
			vid++;
		}
		else if (line[0] == 'f') {
			readFace(line, mesh, fid);
			fid++;
		}
	}

	ifs.close();
	return mesh;
}

GPUMesh* loadObjGPU(std::string path) {
	std::ifstream ifs(path);

	if (!ifs.is_open()) {
		return nullptr;
	}
	GPUMesh* mesh = new GPUMesh();
	std::string line;
	int vid = 1;
	int fid = 1;
	while (std::getline(ifs, line)) {

		if (line[0] == 'v' && ( line[1] != 'n' && line[1] != 't')) {
			readVertex(line, mesh, vid);
			vid++;
		}
		else if (line[0] == 'f') {
			readFace(line, mesh, fid);
			fid++;
		}
	}

	ifs.close();
	return mesh;
}
