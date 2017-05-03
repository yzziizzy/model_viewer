 
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
	
	
	face_cnt = sm->faceCnt = sm->indexCnt / 3;
	
	faces = malloc(face_cnt * 3 * sizeof(*faces));
	CHECK_OOM(faces);
	
	// fill in the faces, calculate face normals
	for(i = 0; i < sm->indexCnt; i += 3) {
		Vector* a, *b, *c;
		struct SolidVertex* o;
		Vector norm;
		
		a = sm->vertices + sm->indices[i];
		b = sm->vertices + sm->indices[i+1];
		c = sm->vertices + sm->indices[i+2];
		o = faces + i;
		
		// calculate the face normal
		vTriFaceNormal(a, a+1, a+2, &norm);
		
		// fill in the face info
		vCopy(&a, &o->pos);
		vCopy(&a+1, &o[1].pos);
		vCopy(&a+2, &o[2].pos);
		
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
	
	sm->solid = renderable_Create(GL_TRIANGLES, "staticMesh", opts, faces, face_cnt);
	
	// set status flag
}


static uint64_t edgeLookupHash(void* key) {
	return HT_HashFn(key, sizeof(uint64_t));
}

static int edgeLookupCmp(void* a, void* b) {
	return (uint64_t)a - (uint64_t)b;
}

static inline uint64_t makeEdgeKey(uint32_t a, uint32_t b) {
	return MIN(a, b) | MAX(a, b) << 32; 
}


static void generateWireframe(StaticMesh* sm) {
	
	int i, edge_cnt, edge_alloc;
	
	Vector* edges;
	
	// allocate for the worst case scenario of a bunch of independent triangles
	edge_alloc = sm->faceCnt * 3 * 2;
	edge_cnt = 0;
	
	edges = malloc(edge_alloc * sizeof(*edges));
	CHECK_OOM(edges);
	
	// initialize edge lookup hash table
	sm->edgeLookup = HT_createCustom(0, edgeLookupHash, edgeLookupCmp);
	
	// edges are considered going from lowest to highest index
	for(i = 0; i < sm->indexCnt; i += 3) {
		uint32_t ai, bi, ci;
		uint64_t e_ab, e_bc, e_ca;
		Vector* a, *b, *c, *o;
		Vector norm;
		
		ai = sm->indices[i];
		bi = sm->indices[i+1];
		ci = sm->indices[i+2];
		
		a = sm->vertices + ai;
		b = sm->vertices + bi;
		c = sm->vertices + ci;
	
		e_ab = makeEdgeKey(ai, bi);
		e_bc = makeEdgeKey(ai, bi);
		e_ca = makeEdgeKey(ci, ai);
		
		// TODO: unique edge index table?
		
		if(HT_get(sm->edgeLookup, (void*)e_ab, NULL)) {
			// not found, add edge
			vCopy(a, edges + edge_cnt);
			vCopy(b, edges + edge_cnt + 1);
			
			HT_set(sm->edgeLookup, (void*)e_ab, 1);
			edge_cnt++;
		}

		if(HT_get(sm->edgeLookup, (void*)e_bc, NULL)) {
			// not found, add edge
			vCopy(b, edges + edge_cnt);
			vCopy(c, edges + edge_cnt + 1);
			
			HT_set(sm->edgeLookup, (void*)e_bc, 1);
			edge_cnt++;
		}
		
		if(HT_get(sm->edgeLookup, (void*)e_ab, NULL)) {
			// not found, add edge
			vCopy(c, edges + edge_cnt);
			vCopy(a, edges + edge_cnt + 1);
			
			HT_set(sm->edgeLookup, (void*)e_ca, 1);
			edge_cnt++;
		}
	
	}
	
	// TODO: realloc? 
	
	
	VAOConfig opts[] = {
		// per vertex
		{3, GL_FLOAT}, // position
		{3, GL_FLOAT}, // face_normal
		{0, 0}
	};
	
	sm->wireframe = renderable_Create(GL_LINES, "staticMeshWireframe", opts, edges, edge_cnt);
	
	// TODO: set status flag
}


static void generatePoints(StaticMesh* sm) {

	VAOConfig opts[] = {
		// per vertex
		{3, GL_FLOAT}, // position
		{0, 0}
	};
	
	sm->points = renderable_Create(GL_POINTS, "staticMeshPoints", opts, sm->vertices, sm->vertexCnt);
	
	// TODO: set status flag
}




void staticMesh_RegenMeta(StaticMesh* sm) {
	
	generateSolid(sm);
//	generateWireframe(sm);
//	generatePoints(sm);
	
}



// hacky for now, sloppily recreates index data
StaticMesh* staticMesh_FromOBJ(OBJContents* obj) {
	int i;
	StaticMesh* sm;
	int vertexCnt = 3 * obj->faceCnt;
	
	struct RenderableVertex* vertices = calloc(1, vertexCnt * sizeof(*vertices));
	CHECK_OOM(vertices);
	
	for(i = 0; i < vertexCnt; i++) {
		vCopy(&obj->faces[i].v, &vertices[i].v);
		vCopy(&obj->faces[i].n, &vertices[i].n);
		
		vertices[i].t.u = obj->faces[i].t.x * 65535;
		vertices[i].t.v = obj->faces[i].t.y * 65535;
	}
	
	
	sm = calloc(1, sizeof(*sm));
	CHECK_OOM(sm);
	
	
	
	
	return sm;
}











