#ifndef __EACSMB_STATICMESH_H__
#define __EACSMB_STATICMESH_H__



#include "hash.h"
#include "renderable.h"
#include "bbox.h"


typedef struct StaticMeshVertex {
	Vector v, n;
	struct {
		float u, v;
	} t;
} StaticMeshVertex;


typedef struct StaticMesh {
	
	char ready;
	
	// always GL_TRIANGLES
	VEC(Vector) vertices;
	VEC(Vector) normals; 
	VEC(Vector) texcoords; 
	VEC(uint32_t) vertexIndices;
	VEC(uint32_t) normalIndices;
	VEC(uint32_t) textureIndices;
	
	char hasNormals;
	char hasTexCoords;
	
	char* texPath;
	Texture* texture;
	
	//int vertexCnt;
	int faceCnt;
	//int indexCnt; // = faceCnt * 3
	
	VEC(int*) edges; // indices into vertices
	
	HashTable* edgeLookup;
	
	// adjacency info
	int* triAdjacency; // uses faceCnt;
	
	// vertex-vertex adj
	// vertex-tri adj
	// edge-edge adj
	
	
	// stats
	Vector aabbMin, aabbMax;
	
	// vertex indices
	struct {
		uint32_t p_x, m_x;
		uint32_t p_y, m_y;
		uint32_t p_z, m_z;
	} extremes;

	
	Vector aabbDimensions;
	Vector aabbCenter;
	Vector meanCenter;
	
	
	// rendering stuff
	BoundingBoxVisual bbox;
	
	AxesVisual av;
	
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

void staticMesh_RegenMeta(StaticMesh* sm);
float StaticMesh_setScale(StaticMesh* sm, float newscale);

void initStaticMeshes();
StaticMesh* StaticMeshFromOBJ(OBJContents* obj);
StaticMesh* StaticMesh_LoadOBJ(char* path);
StaticMesh* staticMesh_FromPLY(PLYContents* pc);

void StaticMesh_CalcStats(StaticMesh* sm);

void drawStaticMesh(StaticMesh* m, Matrix* view, Matrix* proj);


#endif // __EACSMB_STATICMESH_H__
