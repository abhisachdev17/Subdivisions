## Introducion

This source code contains the implementation of Catmull-Clark subdivision on GPU along with the CPU implementation. Detailed explanation of the implementation can be found in the written report provided with the source code.

### How to Run?

#### For GPU MODE

- Launch the program by opening opengl.sln file.
- Build and run the program.

#### For CPU MODE

- Launch the program by opening opengl.sln file.
- navigate to render.cpp file, comment out line 16 #define GPU_MODE
- Build and run the program.

#### Change the obj file

- The program loads cube.obj file by default. To change the OBJ_FILE constant on line 20 to read the appropriate file

### How to Use?

- Use `spacebar` to perform the subdivision on the currently displayed geometry.
- Use key `e` to go back to previous level of subdivision.
- Use key `r` to go to the next level of subdivision.
- Use key `w` to zoom camera in.
- Use key `s` to zoom camera out.
- Use key `a` to move camera left. (translate in negative x)
- Use key `d` to move camera right. (translate in positive x)
- Use key `z` to move camera up. (translate in positive y)
- Use key `d` to move camera down. (translate in positive y)
- Click the `left mouse button` to rotate the view around the Y axis.
- Click the `middle mouse button` to rotate the view around the X axis.
- Click the `right mouse button` to rotate the view around the Z axis.

---

## Important Files

- `render.cpp` contains the rendering code for drawing the geometry.
- `subdivision.cpp` contains the code to perform subdivision for both GPU and CPU implementations.
- `objloader.cpp` is where the obj files are read.
- `cckernel.glsl` is the compute shader for Catmull-Clark subdivision.

## Design Considerations

- 2 VAOs are used to store geometry.
- Each VAO has one associated VBO which stores vertex data.
- First VAO stores geometry for filled faces
- Second VAO stores geometry for outlines.

---

## Third party

- The program needs the freeglut, glew, glm and openGL libraries which are bundled with this code.
- Obj files by Keenan Crane: https://www.cs.cmu.edu/~kmcrane/Projects/ModelRepository/

## Platform

- Operating System: Windows 10 Pro, 64 bit
- Graphics Hardware: NVIDIA RTX 2060
