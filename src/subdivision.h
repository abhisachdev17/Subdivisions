#pragma once
#include "types.h"
#include "gputypes.h"
#include "common.h"

Mesh* cc_subdivide(Mesh* m, GLuint * program);
GPUMesh* cc_subdivide(GPUMesh* m, GLuint * computeProgram);
