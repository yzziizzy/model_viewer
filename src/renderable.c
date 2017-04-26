 
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "ds.h"
#include "hash.h"

#include "c3dlas/c3dlas.h"

#include "utilities.h"
#include "objloader.h"t
#include "shader.h"
#include "renderable.h"








Renderable* renderable_Create(GLuint type, char* shaderPath, VAOConfig* vaoOpts, void* data, int dataCnt) {
	
	Renderable* r;
	int stride;
	int i;
	int offset = 0;
	
	
	// allocation and init
	r = calloc(1, sizeof(*r));
	if(!r) {
		fprintf(stderr, "OOM\n");
		exit(2);
	}
	
	r->data = data;
	r->dataCnt = dataCnt;
	r->type = type;
	

	// shader
	r->prog = loadCombinedProgram(shaderPath);
	glexit("Renderable shader");
	if(!r->prog) {
		printf("Failed to load shader '%s'\n", shaderPath);
	}
	else {
		r->model_ul = glGetUniformLocation(r->prog->id, "mModel");
		r->view_ul = glGetUniformLocation(r->prog->id, "mView");
		r->proj_ul = glGetUniformLocation(r->prog->id, "mProj");
		glexit("Renderable matrix uniforms");
	}
	
	
	// vertex data
	r->vao = makeVAO(vaoOpts);
	glexit("Renderable VAO");
	
	glBindVertexArray(r->vao);
	
	glGenBuffers(1, &r->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
	
	stride = getVAOStride(vaoOpts);
	
	for(i = 0; vaoOpts[i].sz != 0; i++) {
		glEnableVertexAttribArray(i);
		glVertexAttribPointer(i, vaoOpts[i].sz, vaoOpts[i].type, GL_FALSE, stride, offset);
		glexit("Renderable vertex attribs");
		
		offset += getVAOItemSize(&vaoOpts[i]);
	}
	
	glBufferData(GL_ARRAY_BUFFER, dataCnt * stride, data, GL_STATIC_DRAW);
	glexit("Renderable buffer data");

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glexit("static mesh vbo load");
	
	return r;
}

void renderable_Draw(Renderable* r, Matrix* view, Matrix* proj) {
	
	Matrix model;
	
	//mFastMul(view, proj, &mvp);
	mIdent(&model);
	mScale3f(r->scale, r->scale, r->scale, &model);
	mTransv(&r->pos, &model);
	
	glUseProgram(r->prog->id);

	glUniformMatrix4fv(r->model_ul, 1, GL_FALSE, &model.m);
	glUniformMatrix4fv(r->view_ul, 1, GL_FALSE, &view->m);
	glUniformMatrix4fv(r->proj_ul, 1, GL_FALSE, &proj->m);
// 	glUniform3f(color_ul, .5, .2, .9);
	
	glBindVertexArray(r->vao);
	glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
	glexit("Renderable vbo");
	
	glDrawArrays(r->type, 0, r->dataCnt);
	glexit("Renderable draw");
	
}




struct RenderableOBJVertex {
	Vector v, n;
	struct {
		unsigned short u, v;
	} t;
};


Renderable* renderable_FromOBJ(OBJContents* obj) {
	int i;
	VAOConfig opts[] = {
		// per vertex
		{3, GL_FLOAT}, // position
		{3, GL_FLOAT}, // normal
		{2, GL_UNSIGNED_SHORT}, // tex
		
		{0, 0}
	};
	
	int vertexCnt = 3 * obj->faceCnt;
	
	struct RenderableOBJVertex* vertices = calloc(1, vertexCnt * sizeof(*vertices));
	
	for(i = 0; i < vertexCnt; i++) {
		vCopy(&obj->faces[i].v, &vertices[i].v);
		vCopy(&obj->faces[i].n, &vertices[i].n);
		
		vertices[i].t.u = obj->faces[i].t.x * 65535;
		vertices[i].t.v = obj->faces[i].t.y * 65535;
	}
	
	return renderable_Create(
		GL_TRIANGLES,
		"staticMesh",
		opts,
		vertices,
		3 * obj->faceCnt);
	
}



