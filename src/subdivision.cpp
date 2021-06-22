#include "subdivision.h"
#include "types.h"
#include "gputypes.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "common.h"
#include <chrono>

void addFaceToMesh(Mesh* m, Vertex *v1, Vertex* v2, Vertex* v3, Vertex* v4, int id) {
	Vertex** vs = new Vertex * [4];
	vs[0] = v1;
	vs[1] = v2;
	vs[2] = v3;
	vs[3] = v4;

	Face* face = new Face(id, vs);
	m->addFace(face);
}


int getEdge(std::vector <Edge> edges, int v1, int v2) {
	for (int i = 0; i < edges.size(); i++) {

		if (edges[i].vids[0] == v1 || edges[i].vids[1] == v1) {
			if (edges[i].vids[0] == v2 || edges[i].vids[1] == v2) {
				return i;
			}
		}
	}
	return -1;
}

Face* getSharingFace(Mesh* m, int f, int v1, int v2) {
	for (int i = 0; i < m->faces.size(); i++) {
		Face* face = m->faces[i];
		if (face->id != f) {
			bool v1in = false;
			bool v2in = false;
			for (int v = 0; v < 4; v++) {
				if (v1 == face->vertices[v]->id) {
					v1in = true;
				}
				if (v2 == face->vertices[v]->id) {
					v2in = true;
				}
			}
			if (v1in && v2in) {
				return face;
			}
		}
	}
	return nullptr;
}

glm::vec3 calc_edgepoints(glm::vec3 v1, glm::vec3 v2, glm::vec3 f1, glm::vec3 f2) {
	glm::vec3 avg = (v1 + v2 + f1 + f2) / 4.0f;
	return avg;
}

glm::vec3 calc_new_vertex(Vertex* v, Vertex** facepoints) {
	glm::vec3 vdash, S, R;
	float n = v->faces.size();
	for (int i = 0; i < v->faces.size(); i++) {
		S += facepoints[v->faces[i]->id - 1]->vertex;
	}
	S /= n;

	for (int i = 0; i < v->edges.size(); i++) {
		R += (v->vertex + v->edges[i]->vertex) / 2.0f;
	}
	R /= n;

	vdash = (n-3) * v->vertex + 2.0f * R + S;
	return vdash / n;
}

Mesh* cc_subdivide(Mesh* m, GLuint * program) {

	long long start = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()).count();

	Mesh* subdivided = new Mesh();

	std::vector <Edge> edges;
	Vertex** facepoints = new Vertex*[m->faces.size()];

	int curr_id = 1;

	for (int i = 0; i < m->faces.size(); i++) {
		Face* face = m->faces[i];
		glm::vec3 fp = face->facepoint;

		Vertex* fv = new Vertex(curr_id++, fp);
		facepoints[face->id - 1] = fv;
		subdivided->addVertex(fv);
	}

	for (int i = 0; i < m->faces.size(); i++) {
		Face* face = m->faces[i];
		for (int e = 0; e < 4; e++) {
			Vertex* v1 = face->vertices[e];
			Vertex* v2 = face->vertices[(e + 1) % 4];

			Edge edge;
			int edgeidx = getEdge(edges, v1->id, v2->id);
			Vertex* edgeV;
			if (edgeidx == -1) {
				Edge new_edge;
				new_edge.vids[0] = v1->id;
				new_edge.vids[1] = v2->id;
				Face* shared = getSharingFace(m, face->id, v1->id, v2->id);
				new_edge.edgepoint = calc_edgepoints(v1->vertex, v2->vertex, face->facepoint, shared->facepoint);
				
				edgeV = new Vertex(curr_id++, new_edge.edgepoint);
				subdivided->addVertex(edgeV);

				new_edge.e = edgeV;
				edges.push_back(new_edge);

				edge = new_edge;
			}
		}
	}

	int curr_fid = 1;
	Vertex** newVertices = new Vertex * [m->vertices.size()];

	for (int i = 0; i < m->vertices.size(); i++) {
		newVertices[i] = nullptr;
	}

	for (int i = 0; i < m->faces.size(); i++) {
		Face* face = m->faces[i];
		for (int e = 0; e < 4; e++) {
			Vertex* v1 = face->vertices[e];
			Vertex* v2 = face->vertices[(e + 1) % 4];

			int edge = getEdge(edges, v1->id, v2->id);
			Vertex* v3 = face->vertices[(e + 2) % 4];

			int nextedge = getEdge(edges, v2->id, v3->id);
			if (newVertices[v2->id - 1] == nullptr) {
				Vertex* newV = new Vertex(curr_id++, calc_new_vertex(v2, facepoints));
				newVertices[v2->id - 1] = newV;
				subdivided->addVertex(newV);
			}
			
			addFaceToMesh(subdivided, edges[edge].e, facepoints[face->id - 1], edges[nextedge].e, newVertices[v2->id-1], curr_fid++);
		}
	}

	long long end = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()).count();


	std::cout << "Time taken to perform subdivision on CPU: " << end - start << "ms" << std::endl;

	return subdivided;
}


void addFaceToMesh(GPUMesh* m, int v1, int v2, int v3, int v4, int id) {
	GPUFace f;
	f.id = id;
	f.vertices[0] = v1;
	f.vertices[1] = v2;
	f.vertices[2] = v3;
	f.vertices[3] = v4;

	m->addFace(f);
}


int getEdge(std::vector <GPUEdge> edges, int v1, int v2) {
	for (int i = 0; i < edges.size(); i++) {
		if (edges[i].vertices[0] == v1 || edges[i].vertices[1] == v1) {
			if (edges[i].vertices[0] == v2 || edges[i].vertices[1] == v2) {
				return i;
			}
		}
	}
	return -1;
}

GPUMesh* cc_subdivide(GPUMesh* m, GLuint * program) {
	GPUMesh* subdivided = new GPUMesh();
	GPUEdge new_edge;

	// Create edges, they are needed as input for subdivision calculation
	int curr_id = m->faces.size() + 1;
	for (int i = 0; i < m->faces.size(); i++) {
		GPUFace &face = m->faces[i];
		for (int e = 0; e < 4; e++) {
			GPUVertex &v1 = m->vertices[face.vertices[e] - 1];
			GPUVertex &v2 = m->vertices[face.vertices[(e + 1) % 4] - 1];

			int edgeidx = getEdge(m->edges, v1.id, v2.id);
			if (edgeidx == -1) {
				new_edge.vertices[0] = v1.id;
				new_edge.vertices[1] = v2.id;
				new_edge.face = face.id;
				new_edge.id = curr_id++;
				m->edges.push_back(new_edge);
			}
		}
	}

	GLuint uKT = glGetUniformLocation(program[0], "kernelType");
	GLuint uFO = glGetUniformLocation(program[0], "FACE_OFFSET");
	GLuint uEO = glGetUniformLocation(program[0], "EDGE_OFFSET");

	long long start = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()).count();

	// load all data in the GPU
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, program[1]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUVertex) * m->vertices.size(), &m->vertices[0], GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, program[1]);
	
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, program[2]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUFace) * m->faces.size(), &m->faces[0], GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, program[2]);
	
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, program[3]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUVertex) * (m->faces.size() + m->edges.size() + m->vertices.size()), NULL, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, program[3]);
	
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, program[4]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUFace) * (m->faces.size() * 4), NULL, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, program[4]);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, program[5]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUEdge) * m->edges.size(), &m->edges[0], GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, program[5]);

	glUseProgram(program[0]);
	glUniform1i(uFO, m->faces.size());
	glUniform1i(uEO, m->edges.size());

	glUniform1i(uKT, 0); // face point calculation
	glDispatchCompute(m->faces.size(), 1, 1);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, program[2]);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	// Compute the Edgepoints
	glUniform1i(uKT, 1);
	glDispatchCompute(m->edges.size(), 1, 1);

	// Compute the Vertices in parallel
	glUniform1i(uKT, 2);
	glDispatchCompute(m->vertices.size(), 1, 1);

	// wait for the above to finish
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, program[3]);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	// create new subdivided mesh
	glUniform1i(uKT, 3);
	glDispatchCompute(1, 1, 1);
	
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, program[3]);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, program[4]);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	long long end = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()).count();

	std::cout << "Time taken to perform subdivision on GPU: " << end - start << "ms" << std::endl;

	start = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()).count();

	GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	// read the new mesh for rendering
	std::copy((GPUFace*)p, (GPUFace*)p + m->faces.size() * 4, std::back_inserter(subdivided->faces));
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, program[3]);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	std::copy((GPUVertex*)p, (GPUVertex*)p + m->faces.size() + m->edges.size() + m->vertices.size(), std::back_inserter(subdivided->vertices));
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	end = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()).count();

	std::cout << "Time taken to copy buffers to CPU: " << end - start << "ms" << std::endl;
	return subdivided;
}