#ifndef __EACSMB_STATICMESH_H__
#define __EACSMB_STATICMESH_H__



#include "hash.h"
#include "renderable.h"


typedef struct StaticMeshVertex {
	Vector v, n;
	struct {
		unsigned short u, v;
	} t;
} StaticMeshVertex;


typedef struct StaticMesh {
	
	char ready;
	
	// always GL_TRIANGLES
	Vector* vertices;
	Vector* normals; // uses vertexCnt
	int* indices;
	int vertexCnt;
	int faceCnt;
	int indexCnt; // = faceCnt * 3
	
	int* edges; // indices into vertices
	int edgeCnt;
	
	HashTable* edgeLookup;
	
	// adjacency info
	int* triAdjacency; // uses faceCnt;
	
	// vertex-vertex adj
	// vertex-tri adj
	// edge-edge adj
	
	Renderable* solid;
	Renderable* wireframe;
	Renderable* points;
	
	char showSolid;
	char showWireframe;
	char showPoints;
	
	Vector pos;
	Vector raxis;
	float rtheta;
	float scale;
	
	Matrix composed;
	
} StaticMesh;



StaticMesh* staticMesh_Create();
void staticMesh_Destroy(StaticMesh** sm);

void initStaticMeshes();
StaticMesh* StaticMeshFromOBJ(OBJContents* obj);

void drawStaticMesh(StaticMesh* m, Matrix* view, Matrix* proj);


#endif // __EACSMB_STATICMESH_H__
