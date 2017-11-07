#ifndef __EACSMB_RENDERABLE_H__
#define __EACSMB_RENDERABLE_H__

#include "ds.h"

#include "plyloader.h"


typedef struct RenderableVertex {
	Vector v, n;
	struct {
		unsigned short u, v;
	} t;
} RenderableVertex;


typedef struct Renderable {
	
	// old stuff
	GLenum type; // eg, GL_TRIANGLES
	void* data;
	int dataCnt;
	
	GLuint vbo;
	GLuint vao;
	
	char* shaderPath;
	ShaderProgram* prog;
	GLuint model_ul, view_ul, proj_ul;
	
	GLuint texID;
	
	Vector pos;
	Vector raxis;
	float rtheta;
	float scale;
	
	Matrix composed;
	
	
} Renderable;


Renderable* renderable_FromOBJ(OBJContents* obj);
void renderable_Draw(Renderable* r, Matrix* view, Matrix* proj);
Renderable* renderable_Create(GLuint type, char* shaderPath, VAOConfig* vaoOpts, void* data, int dataCnt);


#endif // __EACSMB_RENDERABLE_H__
 
