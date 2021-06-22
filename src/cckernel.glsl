#version 440

struct Vertex {
	float x, y, z;
};

struct GPUVertex
{ 
  int id;
  int currentFace;
  int currentEdge;
  int faces[10]; // max 10 connected faces
  int edges[20]; // max 20 connected edges
  Vertex v;
};

struct GPUFace
{ 
  int id;
  int vertices[4]; 
  Vertex facepoint;
};

struct GPUEdge {
	int id;
	int face;
	int vertices[2];
};

layout(local_size_x = 1, local_size_y = 1) in;

layout (std430, binding = 1) buffer InputVertices
{ 
  GPUVertex vertices[];

} inputVertices;

layout (std430, binding = 2) buffer InputFaces
{ 
  GPUFace faces[];

} inputFaces;

layout (std430, binding = 3) buffer OutputVertices
{ 
  GPUVertex vertices[];

} outputVertices;

layout (std430, binding = 4) buffer OutputFaces
{ 
  GPUFace faces[];

} outputFaces;

layout (std430, binding = 5) buffer Edges
{ 
  GPUEdge edges[];

} inEdges;

uniform int kernelType;
uniform int FACE_OFFSET;
uniform int EDGE_OFFSET;

void faceCalc() {
	Vertex facepoint;

	for (int i = 0; i < 4; i++) {
		facepoint.x += inputVertices.vertices[inputFaces.faces[gl_GlobalInvocationID.x].vertices[i] - 1].v.x;
		facepoint.y += inputVertices.vertices[inputFaces.faces[gl_GlobalInvocationID.x].vertices[i] - 1].v.y;
		facepoint.z += inputVertices.vertices[inputFaces.faces[gl_GlobalInvocationID.x].vertices[i] - 1].v.z;
	}

	facepoint.x /= 4.0;
	facepoint.y /= 4.0;
	facepoint.z /= 4.0;

	GPUVertex newV;
	newV.id = int(gl_GlobalInvocationID.x + 1);
	
	newV.v.x = facepoint.x;
	newV.v.y = facepoint.y;
	newV.v.z = facepoint.z;
	
	outputVertices.vertices[gl_GlobalInvocationID.x] = newV;
	inputFaces.faces[gl_GlobalInvocationID.x].facepoint = facepoint;
}

int getSharedFace(int f, int v1, int v2) {
	for (int i = 0; i < FACE_OFFSET; i++) {
		GPUFace face = inputFaces.faces[i];
		if (face.id != f) {
			bool v1in = false;
			bool v2in = false;
			for (int v = 0; v < 4; v++) {
				if (v1 == face.vertices[v]) {
					v1in = true;
				}
				if (v2 == face.vertices[v]) {
					v2in = true;
				}
			}
			if (v1in && v2in) {
				return face.id;
			}
		}
	}
	return -1;
}

Vertex calc_edgepoints(Vertex v1, Vertex v2, Vertex v3, Vertex v4) {
	Vertex avg;
	avg.x = (v1.x + v2.x + v3.x + v4.x) / 4.0f;
	avg.y = (v1.y + v2.y + v3.y + v4.y) / 4.0f;
	avg.z = (v1.z + v2.z + v3.z + v4.z) / 4.0f;

	return avg;
}

void edgeCalc() {
	GPUEdge e = inEdges.edges[gl_GlobalInvocationID.x];
	GPUVertex v1 = inputVertices.vertices[e.vertices[0] - 1];
	GPUVertex v2 = inputVertices.vertices[e.vertices[1] - 1];
	GPUFace f = inputFaces.faces[e.face - 1];

	GPUVertex edgeV;
	edgeV.id = e.id;

	int sharedFaceIndex = getSharedFace(e.face, v1.id, v2.id);
	if (sharedFaceIndex != -1) {
		GPUFace sharedFace = inputFaces.faces[ sharedFaceIndex - 1];
		edgeV.v = calc_edgepoints(v1.v, v2.v, f.facepoint, sharedFace.facepoint);
	}
	else {
		Vertex err;
		edgeV.v = calc_edgepoints(v1.v, v2.v, err, err);
	}
	outputVertices.vertices[FACE_OFFSET + gl_GlobalInvocationID.x] = edgeV;
}


Vertex calc_new_vertex(GPUVertex v) {
	Vertex vdash, S, R;
	float n = v.currentFace;
	for (int i = 0; i < v.currentFace; i++) {
		S.x += outputVertices.vertices[v.faces[i] - 1].v.x;
		S.y += outputVertices.vertices[v.faces[i] - 1].v.y;
		S.z += outputVertices.vertices[v.faces[i] - 1].v.z;
	}
	S.x /= n;
	S.y /= n;
	S.z /= n;

	for (int i = 0; i < v.currentEdge; i++) {
		R.x += (v.v.x + inputVertices.vertices[v.edges[i] - 1].v.x) / 2.0f;
		R.y += (v.v.y + inputVertices.vertices[v.edges[i] - 1].v.y) / 2.0f;
		R.z += (v.v.z + inputVertices.vertices[v.edges[i] - 1].v.z) / 2.0f;
	}
	R.x /= n;
	R.y /= n;
	R.z /= n;

	vdash.x = (n - 3) * v.v.x + 2.0f * R.x + S.x;
	vdash.y = (n - 3) * v.v.y + 2.0f * R.y + S.y;
	vdash.z = (n - 3) * v.v.z + 2.0f * R.z + S.z;

	vdash.x /= n;
	vdash.y /= n;
	vdash.z /= n;

	return vdash;
}

void vertCalc() {
	GPUVertex newV;
	newV.id = int(FACE_OFFSET + EDGE_OFFSET + gl_GlobalInvocationID.x + 1);
	newV.v = calc_new_vertex(inputVertices.vertices[gl_GlobalInvocationID.x]);
	outputVertices.vertices[newV.id - 1] = newV;
}


int getEdgeIndex(GPUVertex v, int edge) {
	for (int i = 0; i < v.currentEdge; i++) {
		if (v.edges[i] == edge) {
			return i;
		}
	}

	return -1;
}

void addEdge(GPUVertex v, int edge) {
	if (getEdgeIndex(v, edge) == -1) {
		v.edges[v.currentEdge] = edge;
		v.currentEdge++;
		outputVertices.vertices[v.id - 1] = v;
	}
}

void addFaceToMesh(int v1, int v2, int v3, int v4, int id) {
	GPUFace f;
	f.id = id;
	f.vertices[0] = v1;
	f.vertices[1] = v2;
	f.vertices[2] = v3;
	f.vertices[3] = v4;

	outputFaces.faces[id-1] = f;

	for (int i = 0; i < 4; i++) {
		GPUVertex v = outputVertices.vertices[f.vertices[i] - 1];
		v.faces[v.currentFace] = f.id;
		v.currentFace++;
		outputVertices.vertices[f.vertices[i] - 1] = v;
	}

	for (int i = 0; i < 4; i++) {
		GPUVertex v = outputVertices.vertices[f.vertices[i] - 1];
		int edge = outputVertices.vertices[f.vertices[(i + 1) % 4] - 1].id;
		addEdge(v, edge);
		
		int check = i - 1;
		if ((i - 1) < 0) {
			check = 3;
		}

		edge = outputVertices.vertices[f.vertices[check] - 1].id;
		addEdge(v, edge);
	}

}


int getEdge(int v1, int v2) {
	for (int i = 0; i < EDGE_OFFSET; i++) {
		if (inEdges.edges[i].vertices[0] == v1 || inEdges.edges[i].vertices[1] == v1) {
			if (inEdges.edges[i].vertices[0] == v2 || inEdges.edges[i].vertices[1] == v2) {
				return i;
			}
		}
	}
	return -1;
}

void faceAdder() {
	int offset = FACE_OFFSET + EDGE_OFFSET;
	int fid = 1;
	for (int i=0; i < FACE_OFFSET; i++) {
		GPUFace face = inputFaces.faces[i];
		for (int e = 0; e < 4; e++) {
			GPUVertex v1 = inputVertices.vertices[face.vertices[e] - 1];
			GPUVertex v2 = inputVertices.vertices[face.vertices[(e + 1) % 4] - 1];

			int edge = getEdge(v1.id, v2.id);
			GPUVertex v3 = inputVertices.vertices[face.vertices[(e + 2) % 4] - 1];

			int nextedge = getEdge(v2.id, v3.id);
			addFaceToMesh(inEdges.edges[edge].id, outputVertices.vertices[face.id - 1].id, inEdges.edges[nextedge].id, outputVertices.vertices[offset + v2.id - 1].id, fid++);
		}
	}
}


void main() {
	if (kernelType == 0) {
		faceCalc();
	}
	else if(kernelType == 1) {
		edgeCalc(); 
	}
	else if(kernelType == 2) {
		vertCalc();
	}
	else if (kernelType == 3) {
		faceAdder();
	}
}

