 
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


int static parseFaceVertex(char** s, int* info);


struct SolidVertex {
	Vector pos;
	Vector face_normal;
	struct {
		unsigned short u, v;
	} texcoords;
};

static HASH_FN_FOR_TYPE(vector_hash_fn, Vector)


static void generateSolid(StaticMesh* sm) {
	
	int i, face_cnt;
	
	struct SolidVertex* faces;
	
	
	face_cnt = sm->faceCnt = VEC_LEN(&sm->vertexIndices) / 3;
	
	faces = malloc(face_cnt * 3 * sizeof(*faces));
	CHECK_OOM(faces);
	
	// fill in the faces, calculate face normals
	for(i = 0; i < VEC_LEN(&sm->vertexIndices); i += 3) {
		Vector* a, *b, *c;
		Vector* an, *bn, *cn;
		struct SolidVertex* o;
		Vector norm;
		
		a = VEC_DATA(&sm->vertices) + VEC_DATA(&sm->vertexIndices)[i];
		b = VEC_DATA(&sm->vertices) + VEC_DATA(&sm->vertexIndices)[i+1];
		c = VEC_DATA(&sm->vertices) + VEC_DATA(&sm->vertexIndices)[i+2];

		an = VEC_DATA(&sm->normals) + VEC_DATA(&sm->normalIndices)[i];
		bn = VEC_DATA(&sm->normals) + VEC_DATA(&sm->normalIndices)[i+1];
		cn = VEC_DATA(&sm->normals) + VEC_DATA(&sm->normalIndices)[i+2];

		o = faces + i;
		
		// calculate the face normal
		//vTriFaceNormal(a, b, c, &norm);
		
		//printf("face %d: [%.2f,%.2f,%.2f] [%.2f,%.2f,%.2f] [%.2f,%.2f,%.2f] \n", i/3, a->x, a->y, a->z, b->x, b->y, b->z, c->x, c->y, c->z);
		// fill in the face info
		vCopy(a, &o->pos);
		vCopy(b, &o[1].pos);
		vCopy(c, &o[2].pos);
		
		vCopy(an, &o->face_normal);
		vCopy(bn, &(o+1)->face_normal);
		vCopy(cn, &(o+2)->face_normal);
		
		o->texcoords.u = VEC_DATA(&sm->texcoords)[VEC_DATA(&sm->textureIndices)[i]].x * 65536;
		o->texcoords.v = VEC_DATA(&sm->texcoords)[VEC_DATA(&sm->textureIndices)[i]].y * 65536;
		(o+1)->texcoords.u = VEC_DATA(&sm->texcoords)[VEC_DATA(&sm->textureIndices)[i+1]].x * 65536;
		(o+1)->texcoords.v = VEC_DATA(&sm->texcoords)[VEC_DATA(&sm->textureIndices)[i+1]].y * 65536;
		(o+2)->texcoords.u = VEC_DATA(&sm->texcoords)[VEC_DATA(&sm->textureIndices)[i+2]].x * 65536;
		(o+2)->texcoords.v = VEC_DATA(&sm->texcoords)[VEC_DATA(&sm->textureIndices)[i+2]].y * 65536;
	}
	
	
	VAOConfig opts[] = {
		// per vertex
		{3, GL_FLOAT}, // position
		{3, GL_FLOAT}, // face_normal
		{2, GL_UNSIGNED_SHORT, GL_TRUE}, // tex
		{0, 0}
	};
	
	sm->solid = renderable_Create(GL_TRIANGLES, "staticMesh", opts, faces, face_cnt * 3);
	
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
	/*
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
		
		a = VEC_DATA(&sm->vertices) + ai;
		b = VEC_DATA(&sm->vertices) + bi;
		c = VEC_DATA(&sm->vertices) + ci;
	
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
	*/
}


static void generatePoints(StaticMesh* sm) {

	VAOConfig opts[] = {
		// per vertex
		{3, GL_FLOAT}, // position
		{0, 0}
	};
	
	sm->points = renderable_Create(GL_POINTS, "staticMeshPoints", opts, VEC_DATA(&sm->vertices), VEC_LEN(&sm->vertices));
	
	// TODO: set status flag
}




void staticMesh_RegenMeta(StaticMesh* sm) {
	
	generateSolid(sm);
//	generateWireframe(sm);
	generatePoints(sm);
	
}

void staticMesh_Init(StaticMesh* sm) {
	
	VEC_INIT(&sm->vertices);
	VEC_INIT(&sm->normals);
	VEC_INIT(&sm->texcoords);

	VEC_INIT(&sm->vertexIndices);
	VEC_INIT(&sm->normalIndices);
	VEC_INIT(&sm->textureIndices);

	VEC_INIT(&sm->edges);
	
}

StaticMesh* staticMesh_Create() {
	StaticMesh* sm;
	
	sm = calloc(1, sizeof(*sm));
	CHECK_OOM(sm);
	
	staticMesh_Init(sm);
	
	return sm;
}


// hacky for now, sloppily recreates index data
StaticMesh* staticMesh_LoadOBJ(char* path) {
	int i;
	StaticMesh* sm;
	char* f, *raw;
	int chars_read;
	Vector tmp;
	
	int tc, vc, nc;
	tc = vc = nc = 0;
	
	raw = readFile(path, NULL);
	if(!raw) return;
	
	sm = staticMesh_Create();
	
	
	f = raw;
	while(*f) {
		char c;
		int fd[3][4];
		int n, ret;
		
		c = *f;
		f++;
		
		if(c == 'f') {
			
			// this loop decomposes polygons into fans of individual triangles
			ret = parseFaceVertex(&f, &fd[0]); // the center vertex
			ret = parseFaceVertex(&f, &fd[1]); // the next vertex
			do {
				ret = parseFaceVertex(&f, &fd[2]);
				if(ret) break;
				
				VEC_PUSH(&sm->vertexIndices,  fd[0][0]-1);
				VEC_PUSH(&sm->textureIndices, fd[0][1]-1);
				VEC_PUSH(&sm->normalIndices,  fd[0][2]-1);
				
				VEC_PUSH(&sm->vertexIndices,  fd[1][0]-1);
				VEC_PUSH(&sm->textureIndices, fd[1][1]-1);
				VEC_PUSH(&sm->normalIndices,  fd[1][2]-1);
				
				VEC_PUSH(&sm->vertexIndices,  fd[2][0]-1);
				VEC_PUSH(&sm->textureIndices, fd[2][1]-1);
				VEC_PUSH(&sm->normalIndices,  fd[2][2]-1);
				
				// shift the data down
				memcpy(&fd[1], &fd[2], sizeof(fd[2]));
			} while(1);
		}
		else if(c == 'v') {
			
			c = *f;
			f++;
			if(c == ' ') { // vertex
				
				chars_read = 0;
				n = sscanf(f, " %f %f %f%n", &tmp.x, &tmp.y, &tmp.z, &chars_read);
				VEC_PUSH(&sm->vertices, tmp);
				
				if(n < 3) fprintf(stderr, "error reading vertex\n");
				
				f += chars_read-1;
				vc++;
			}
			else if(c == 't') { // tex coord
				chars_read = 0;
				tmp.z = 0.0f;
				n = sscanf(f, " %f %f%n", &tmp.x, &tmp.y, &chars_read);
				VEC_PUSH(&sm->texcoords, tmp);
				
				if(n < 2) fprintf(stderr, "error reading tex coord\n");
				
				f += chars_read-1;
				tc++;
			}
			else if(c == 'n') { // normal 
				chars_read = 0;
				n = sscanf(f, " %f %f %f%n", &tmp.x, &tmp.y, &tmp.z, &chars_read);
				VEC_PUSH(&sm->normals, tmp);
				
				if(n < 3) fprintf(stderr, "error reading normal\n");
				
				f += chars_read-1;
				nc++;
			}
			
		}
		else if(c == 'o') {
			
			
		}
		else if(c == 'm') {
			
			
		}
		
		
		// skip to the end of the line
		while(*f && *f != '\n') f++;
		f++;
	}
	
	printf("static mesh vertices: %d\n", VEC_LEN(&sm->vertices));
	printf("static mesh indices: %d\n", VEC_LEN(&sm->vertexIndices));
	
	return sm;
}


int static parseFaceVertex(char** s, int* info) {
	char* end;
	int val;
	int err, chars_read;
	
	while(**s && (**s == ' ' || **s == '\t' || **s == '\r')) (*s)++;
	if(**s == '\n') return 1;
	
	info[3] = 0;
	
	// first  
	err = strtol_2(*s, &chars_read, &val);
	if(err) return 1;
	*s += chars_read ;
	info[0] = val;
	info[3]++;
	
	if(**s != '/') return 0;
	(*s)++;
	
	err = strtol_2(*s, &chars_read, &val);
	if(err) return 0;
	*s += chars_read ;
	info[1] = val;
	info[3]++;
	
	if(**s != '/') return 0;
	(*s)++;
	
	err = strtol_2(*s, &chars_read, &val);
	if(err) return 0;
	*s += chars_read ;
	info[2] = val;
	info[3]++;
	
	return 0;
}



void drawStaticMesh(StaticMesh* m, Matrix* view, Matrix* proj) {
	
	
	
	m->solid->scale = 1;
	renderable_Draw(m->solid, view, proj);
//	renderable_Draw(m->points, view, proj);
}




