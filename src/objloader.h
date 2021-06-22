#pragma once
#include "types.h"
#include "gputypes.h"
#include <iostream>

Mesh* loadObj(std::string path);
GPUMesh* loadObjGPU(std::string path);