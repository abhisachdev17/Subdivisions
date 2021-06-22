#pragma once
#include <vector>
#include "types.h"
#include <string>

template <class T>
class MeshManager {
private:
	int current_mesh;
	std::vector<T*> meshes;
	static MeshManager<T>* mgr_instance;
	MeshManager() : current_mesh(0) {}

public:
	static MeshManager<T>* instance();
	T* getMesh();
	void previous();
	void next();
	void addMesh(T* mesh);
	T* addMesh(std::string path);

	MeshManager<T>(MeshManager<T>& other) = delete;
	void operator=(const MeshManager<T>&) = delete;
};

template <class T>
MeshManager<T>* MeshManager<T>::mgr_instance = nullptr;

template <class T>
MeshManager<T>* MeshManager<T>::instance() {
	if (!mgr_instance) {
		mgr_instance = new MeshManager();
	}
	return mgr_instance;
}

template <class T>
T* MeshManager<T>::getMesh() {
	return meshes[current_mesh];
}

template <class T>
void MeshManager<T>::addMesh(T* mesh) {
	meshes.push_back(mesh);
}

template <class T>
void MeshManager<T>::next() {
	if (current_mesh < meshes.size() - 1) {
		current_mesh++;
	}
}

template <class T>
void MeshManager<T>::previous() {
	if (current_mesh > 0) {
		current_mesh--;
	}
}