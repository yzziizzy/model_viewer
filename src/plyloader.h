#ifndef __mesh_viewer__plyloader_h__
#define __mesh_viewer__plyloader_h__

#include "common_math.h"

#include "ds.h"



enum PropType {
	PT_LIST,
	PT_FLOAT,
	PT_DOUBLE,
	PT_INT8,
	PT_INT16,
	PT_INT32,
	PT_INT64,
	PT_UINT8,
	PT_UINT16,
	PT_UINT32,
	PT_UINT64,
	PT_MAX_VAL
}


typedef struct {
	char* name;
	enum PropType type;
	enum PropType list_len;
	enum PropType list_item;
	
} ply_prop;


typedef struct {
	char* name;
	long count;
	int stride; // lists don't have this
	
	char hasLists;
	
	VEC(ply_prop) props;
	
} ply_elem;



typedef struct {
	
	char* path;
	
		// these are read from the header
	char isBinary;
	char isBigEndian;
	
	long numVertices;
	long numFaces;
	
	VEC(char*) comments;
	
	char* texPath; // only one texture supported atm
	
	VEC(Vector) vertices;
	VEC(Vector4) colors;
	VEC(Vector) normals; // NYI
	VEC(Vector) texcoords;
	
	VEC(int) faces;
	
	// parsing info
	
	VEC(ply_elem) elements;
	
	// elements
	char foundVertices;
	char foundFaces;

	// properties
	char foundIndices;
	char foundNormals;
	char foundVertTexCoords;
	char foundFaceTexCoords;
	char foundVertColors;
	
} PLYContents;











#endif // __mesh_viewer__plyloader_h__
