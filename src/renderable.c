 
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
		r->tex_ul = glGetUniformLocation(r->prog->id, "tex");
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
	
	glActiveTexture(GL_TEXTURE0 + 5);
	glBindTexture(GL_TEXTURE_2D, r->tex->tex_id);
	
	
	glUseProgram(r->prog->id);

	
	glUniform1i(r->tex_ul, 5);
	
	glUniformMatrix4fv(r->model_ul, 1, GL_FALSE, &model.m);
	glUniformMatrix4fv(r->view_ul, 1, GL_FALSE, &view->m);
	glUniformMatrix4fv(r->proj_ul, 1, GL_FALSE, &proj->m);
// 	glUniform3f(color_ul, .5, .2, .9);
	
	glBindVertexArray(r->vao);
	glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
	glexit("Renderable vbo");
	
	//printf("vertices %d\n", r->dataCnt);
	
	glDrawArrays(r->type, 0, r->dataCnt);
	glexit("Renderable draw");
	
}


int Renderable_applyTexturePath(Renderable* r, char* path) {
	
	r->tex = loadBitmapTexture(path);
	if(!r->tex) return 1;
	
	r->texturePath = path;
	
	return 0;
}



struct RenderableOBJVertex {
	Vector v, n;
	struct {
		float u, v;
	} t;
};


Renderable* renderable_FromPLY(PLYContents* pc) {
	int i;
	Renderable* r;
	VAOConfig opts[] = {
		// per vertex
		{3, GL_FLOAT}, // position
		{3, GL_FLOAT}, // normal
		{2, GL_FLOAT}, // tex
		
		{0, 0}
	};
	
	Vector null = {0,0,0};
	
	//return NULL;
	
	struct RenderableOBJVertex* vertices = calloc(1, VEC_LEN(&pc->faces) * sizeof(*vertices));
	
	for(i = 0; i < VEC_LEN(&pc->faces); i++) {
		int q = VEC_ITEM(&pc->faces, i);
		int r = VEC_ITEM(&pc->indicesTex, i);
		
		if(q >= VEC_LEN(&pc->vertices)) {
			printf("vertex index outside bounds %d: (%d, %d)\n", i, q, VEC_LEN(&pc->vertices));
			continue;
		} 
		
		vCopy(&VEC_ITEM(&pc->vertices, q), &vertices[i].v);
		vertices[i].t.u = VEC_ITEM(&pc->texcoords, r).x;
		vertices[i].t.v = VEC_ITEM(&pc->texcoords, r).y;
		
		vCopy(&null, &vertices[i].n);
		
		//if(vertices[i].v.x != 0.0)
		//printf("[%.8f, %.8f, %.8f]\n", vertices[i].v.x, vertices[i].v.y, vertices[i].v.z);
		
		
		//vertices[i].t.u = obj->faces[i].t.x * 65536;
		//vertices[i].t.v = obj->faces[i].t.y * 65536;
		
		//printf("texcoord: [%f, %f] -> [%d, %d]\n",obj->faces[i].t.x, obj->faces[i].t.y, vertices[i].t.u,vertices[i].t.v );
	}
	
	r = renderable_Create(
		GL_TRIANGLES,
		"staticMesh",
		opts,
		vertices,
		VEC_LEN(&pc->faces));
	
	printf("tex path: '%s'\n", pc->texPath);
	if(pc->texPath) Renderable_applyTexturePath(r, pc->texPath);
	
	return r;
}



Renderable* renderable_FromOBJ(OBJContents* obj) {
	int i;
	VAOConfig opts[] = {
		// per vertex
		{3, GL_FLOAT}, // position
		{3, GL_FLOAT}, // normal
		{2, GL_UNSIGNED_SHORT, GL_TRUE}, // tex
		
		{0, 0}
	};
	
	int vertexCnt = 3 * obj->faceCnt;
	
	struct RenderableOBJVertex* vertices = calloc(1, vertexCnt * sizeof(*vertices));
	
	for(i = 0; i < vertexCnt; i++) {
		vCopy(&obj->faces[i].v, &vertices[i].v);
		vCopy(&obj->faces[i].n, &vertices[i].n);
		
		vertices[i].t.u = obj->faces[i].t.x * 65536;
		vertices[i].t.v = obj->faces[i].t.y * 65536;
		
		//printf("texcoord: [%f, %f] -> [%d, %d]\n",obj->faces[i].t.x, obj->faces[i].t.y, vertices[i].t.u,vertices[i].t.v );
	
	}
	
	return renderable_Create(
		GL_TRIANGLES,
		"staticMesh",
		opts,
		vertices,
		3 * obj->faceCnt);
	
}



