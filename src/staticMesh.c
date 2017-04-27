 
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "c3dlas/c3dlas.h"

#include "utilities.h"
#include "objloader.h"
#include "shader.h"
#include "staticMesh.h"




struct SolidVertex {
	Vector pos;
	Vector face_normal;
};

static HASH_FN_FOR_TYPE(vector_hash_fn, Vector)


static void generateSolid(StaticMesh* sm) {
	
	int i, face_cnt;
	
	struct SolidVertex* faces;
	
	
	face_cnt = sm->faceCnt = sm->vertexCnt / 3;
	
	faces = malloc(face_cnt * 3 * sizeof(*faces));
	if(!faces) {
		printf("OOM. buy more ram.\n");
		exit(1);
	}
	
	// fill in the faces, calculate face normals
	for(i = 0; i < sm->vertexCnt; i += 3) {
		Vector* a, *b, *c, *o;
		Vector norm;
		
		a = sm->vertices + sm->indices[i];
		b = sm->vertices + sm->indices[i+1];
		c = sm->vertices + sm->indices[i+2];
		o = faces + i;
		
		// calculate the face normal
		vTriFaceNormal(a, a+1, a+2, &norm);
		
		// fill in the face info
		vCopy(&a, &o->pos);
		vCopy(&(a+1), &(o+1)->pos);
		vCopy(&(a+2), &(o+2)->pos);
		
		vCopy(&norm, &o->face_normal);
		vCopy(&norm, &(o+1)->face_normal);
		vCopy(&norm, &(o+2)->face_normal);
	}
	
	
	VAOConfig opts[] = {
		// per vertex
		{3, GL_FLOAT}, // position
		{3, GL_FLOAT}, // face_normal
		{0, 0}
	};
	
	sm->solid = renderable_Create(GL_TRIANGLES, "staticMesh", opts, faces, faceCnt);
	
	// set status flag
}

static void generateWireframe(StaticMesh* sm) {
	
	
	
}


static void generatePoints(StaticMesh* sm) {
	
	
	
}



















