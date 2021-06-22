/**
 render.cpp

 Contains all the opengl render code
*/
#include "common.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "types.h"
#include "gputypes.h"
#include <vector>
#include "subdivision.h"
#include "objloader.h"
#include "meshmgr.h"
#define GPU_MODE //comment this out to run CPU subdivision

const char *WINDOW_TITLE = "Catmull Clark Subdivision";
const double FRAME_RATE_MS = 1000.0/60.0;
const char* OBJ_FILE = "./assets/cube.obj";

#ifdef GPU_MODE
MeshManager<GPUMesh>* meshMgr = MeshManager<GPUMesh>::instance();
#else
MeshManager<Mesh>* meshMgr = MeshManager<Mesh>::instance();
#endif

// Array of rotation angles (in degrees) for each coordinate axis
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
GLfloat  Theta[NumAxes] = { 0.0, 0.0, 0.0 };
bool rotateX = false;
bool rotateY = false;
bool rotateZ = false;

float xd = 0.0, yd = 0.0, zd = 5.0;

// Model-view and projection matrices uniform location
GLuint  ModelView, Projection, uColor;

glm::vec3* data;
glm::vec3* outline;

int numVertices;
int numVerticesO;

bool outlineOn = true;
bool divide = false;
bool prev = false;
bool next = false;

GLuint vao[2];
GLuint buff[2];
GLuint computeProgram;
GLuint renderProgram;

//----------------------------------------------------------------------------

// quad generates two triangles for each face
int* loadBuffers(Mesh *m) {
    data = new glm::vec3[m->faces.size() * 6];
    outline = new glm::vec3[m->faces.size() * 8];
    int in = 0;
    int jn = 0;

    int* sizes = new int[2];
    for (int i = 0; i < m->faces.size(); i++) {
        Face* f = m->faces[i];
        Vertex* v1 = f->vertices[0];
        Vertex* v2 = f->vertices[1];
        Vertex* v3 = f->vertices[2];
        Vertex* v4 = f->vertices[3];

        data[in++] = v1->vertex;
        data[in++] = v2->vertex;
        data[in++] = v3->vertex;
        data[in++] = v1->vertex;
        data[in++] = v3->vertex;
        data[in++] = v4->vertex;

        outline[jn++] = v1->vertex;
        outline[jn++] = v2->vertex;
        outline[jn++] = v2->vertex;
        outline[jn++] = v3->vertex;
        outline[jn++] = v3->vertex;
        outline[jn++] = v4->vertex;
        outline[jn++] = v4->vertex;
        outline[jn++] = v1->vertex;
    }
    sizes[0] = in;
    sizes[1] = jn;


    return sizes;
}


// quad generates two triangles for each face
int* loadBuffers(GPUMesh* m) {
    data = new glm::vec3[m->faces.size() * 6];
    outline = new glm::vec3[m->faces.size() * 8];
    int in = 0;
    int jn = 0;
    
    int* sizes = new int[2];
    for (int i = 0; i < m->faces.size(); i++) {
        GPUFace f = m->faces[i];
        GPUVertex v1 = m->vertices[f.vertices[0] - 1];
        GPUVertex v2 = m->vertices[f.vertices[1] - 1];
        GPUVertex v3 = m->vertices[f.vertices[2] - 1];
        GPUVertex v4 = m->vertices[f.vertices[3] - 1];

        data[in++] = glm::vec3(v1.vertex.x, v1.vertex.y, v1.vertex.z);
        data[in++] = glm::vec3(v2.vertex.x, v2.vertex.y, v2.vertex.z);
        data[in++] = glm::vec3(v3.vertex.x, v3.vertex.y, v3.vertex.z);
        data[in++] = glm::vec3(v1.vertex.x, v1.vertex.y, v1.vertex.z);
        data[in++] = glm::vec3(v3.vertex.x, v3.vertex.y, v3.vertex.z);
        data[in++] = glm::vec3(v4.vertex.x, v4.vertex.y, v4.vertex.z);

        outline[jn++] = glm::vec3(v1.vertex.x, v1.vertex.y, v1.vertex.z);
        outline[jn++] = glm::vec3(v2.vertex.x, v2.vertex.y, v2.vertex.z);
        outline[jn++] = glm::vec3(v2.vertex.x, v2.vertex.y, v2.vertex.z);
        outline[jn++] = glm::vec3(v3.vertex.x, v3.vertex.y, v3.vertex.z);
        outline[jn++] = glm::vec3(v3.vertex.x, v3.vertex.y, v3.vertex.z);
        outline[jn++] = glm::vec3(v4.vertex.x, v4.vertex.y, v4.vertex.z);
        outline[jn++] = glm::vec3(v4.vertex.x, v4.vertex.y, v4.vertex.z);
        outline[jn++] = glm::vec3(v1.vertex.x, v1.vertex.y, v1.vertex.z);
    }
    sizes[0] = in;
    sizes[1] = jn;

    return sizes;
}

void bufferData(int *sizes) {
    numVertices = sizes[0];
    numVerticesO = sizes[1];

    glUseProgram(renderProgram);
    glBindVertexArray(vao[0]);

    glBindBuffer(GL_ARRAY_BUFFER, buff[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * numVertices, data, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    glBindVertexArray(vao[1]);

    glBindBuffer(GL_ARRAY_BUFFER, buff[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * numVerticesO, outline, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
}

//----------------------------------------------------------------------------
GLuint inputssbo[5];

// OpenGL initialization
void
init()
{
    computeProgram = InitShader("cckernel.glsl");
    glGenBuffers(5, inputssbo);

    // Load shaders and use the resulting shader program
    renderProgram = InitShader("vshader.glsl", "fshader.glsl");
    glUseProgram(renderProgram);

    glGenVertexArrays(2, vao);
    glGenBuffers(2, buff);

#ifdef GPU_MODE
    GPUMesh* m = loadObjGPU(OBJ_FILE);
#else
    Mesh* m = loadObj(OBJ_FILE);
#endif
    
    if (!m) {
        return;
    }  
    meshMgr->addMesh(m);
    int* sizes = loadBuffers(m);
    bufferData(sizes);
    
    // Retrieve transformation uniform variable locations
    ModelView = glGetUniformLocation(renderProgram, "ModelView" );
    Projection = glGetUniformLocation(renderProgram, "Projection" );
    uColor = glGetUniformLocation(renderProgram, "uColor");

    glEnable( GL_DEPTH_TEST );

    glShadeModel(GL_FLAT);

    glClearColor( 1.0, 1.0, 1.0, 1.0 ); 
}

//----------------------------------------------------------------------------

void
display( void )
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //  Generate tha model-view matrix
    const glm::vec3 viewer_pos( xd, yd, zd );
    glm::mat4 trans, rot, scale, model_view;
    trans = glm::translate(trans, -viewer_pos);
    rot = glm::rotate(rot, glm::radians(Theta[Xaxis]), glm::vec3(1,0,0));
    rot = glm::rotate(rot, glm::radians(Theta[Yaxis]), glm::vec3(0,1,0));
    rot = glm::rotate(rot, glm::radians(Theta[Zaxis]), glm::vec3(0,0,1));
    scale = glm::scale(scale, glm::vec3(1.001, 1.001, 1.001));
    model_view = trans * rot;
    
    glUniformMatrix4fv( ModelView, 1, GL_FALSE, glm::value_ptr(model_view) );

    glBindVertexArray(vao[0]);
    glUniform3f(uColor, 1.0, 0.75, 0);
    glDrawArrays( GL_TRIANGLES, 0, numVertices );

    if (outlineOn) {
        glBindVertexArray(vao[1]);
        model_view = trans * scale * rot;
        glUniformMatrix4fv(ModelView, 1, GL_FALSE, glm::value_ptr(model_view));
        glUniform3f(uColor, 0.0, 0.0, 0);
        glDrawArrays(GL_LINES, 0, numVerticesO);
    }
    glutSwapBuffers();
}

//----------------------------------------------------------------------------

void
mouse( int button, int state, int x, int y )
{
    if ( state == GLUT_DOWN ) {
       switch( button ) {
          case GLUT_LEFT_BUTTON:    rotateX = !rotateX;  break;
          case GLUT_MIDDLE_BUTTON:  rotateY = !rotateY;  break;
          case GLUT_RIGHT_BUTTON:   rotateZ = !rotateZ;  break;
       }
    }
}

//----------------------------------------------------------------------------

void reloadBuffers() {
    delete data;
    delete outline;

    int* sizes = loadBuffers(meshMgr->getMesh());
    bufferData(sizes);
}

void
update( void )
{
    if (rotateX) {
        Theta[Xaxis] += 0.5;
    }
    if (rotateY) {
        Theta[Yaxis] += 0.5;
    }
    if (rotateZ) {
        Theta[Zaxis] += 0.5;
    }

    for (int i = 0; i < 3; i++) {
        if (Theta[i] > 360.0) {
            Theta[i] -= 360.0;
        }
    }
    
    if (divide) {
        GLuint* programData = new GLuint[6];
        programData[0] = computeProgram;
        for (int i = 1; i < 6; i++) {
            programData[i] = inputssbo[i - 1];
        }
        meshMgr->addMesh(cc_subdivide(meshMgr->getMesh(), programData));
        meshMgr->next();
        reloadBuffers();
        divide = false;
    }

    if (prev) {
        meshMgr->previous();
        reloadBuffers();
        prev = false;
    }

    if (next) {
        meshMgr->next();
        reloadBuffers();
        next = false;
    }
}

//----------------------------------------------------------------------------

void
keyboard( unsigned char key, int x, int y )
{
    switch( key ) {
	case 033: // Escape Key
	case 'q': case 'Q':
	    exit( EXIT_SUCCESS );
	    break;
    case 'o': case 'O':
        outlineOn = !outlineOn;
        break;
    case ' ':
        divide = true;
        break;

    case 'e': case 'E':
        prev = true;
        break;

    case 'r': case 'R':
        next = true;
        break;

    case 'w': case 'W':
        zd -= 0.1;
        break;

    case 's': case 'S':
        zd += 0.1;
        break;

    case 'a': case 'A':
        xd -= 0.1;
        break;

    case 'd': case 'D':
        xd += 0.1;
        break;

    case 'z': case 'Z':
        yd -= 0.1;
        break;

    case 'X': case 'x':
        yd += 0.1;
        break;

    }
}

//----------------------------------------------------------------------------

void
reshape( int width, int height )
{
    glViewport( 0, 0, width, height );

    GLfloat aspect = GLfloat(width)/height;
    glm::mat4  projection = glm::perspective( glm::radians(45.0f), aspect, 1.0f, 10.0f );

    glUniformMatrix4fv( Projection, 1, GL_FALSE, glm::value_ptr(projection) );
}
