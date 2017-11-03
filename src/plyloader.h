#ifndef __mesh_viewer__plyloader_h__
#define __mesh_viewer__plyloader_h__

#include "common_math.h"

#include "ds.h"



enum PropType {
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
	long count;
	int stride;
	
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
	VEC(Vector) colors;
	VEC(Vector) normals; // NYI
	VEC(Vector) texcoords;
	
	VEC(int) faces;
	
	// parsing info
	
	// byte or position offsets into each element 
	short r_offset, g_offset, b_offset;
	short x_offset, y_offset, z_offset;
	short u_offset, v_offset;
	
	int vertexStride;
	int faceStride;
	
	VEC(ply_elem) elements;
	
} PLYContents;











#endif // __mesh_viewer__plyloader_h__
