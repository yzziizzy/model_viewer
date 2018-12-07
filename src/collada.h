#ifndef COLLADA_H_INCLUDED
#define COLLADA_H_INCLUDED



#include "ds.h"
#include "hash.h"




/*

asset/up_axis/{text}
library_geometries/geometry[id,name]
	/mesh
		/source[id]  (positions)
			/float_array[id,count](text)
			/technique_common/accessor[count, stride]
				/param[name=X,Y,Z, type=float]
		/source ... (normals)
		/source ... (uvs)
		/vertices[id]/input[semantic="position", source="</source[id]>"] 
		/polylist[count]
			/input[semantic=VERTEX,NORMAL,TEXCOORD, source=#id, offset=<int>]
			/vcount(text)
			/p(text)
library_visual_scenes/visual_scene/node[id, name]/matrix[sid=transform]

*/



enum {
	COLLADA_NONE,
	COLLADA_FLOATS,
	COLLADA_INTS
};


typedef struct {
	
	char* id;
	char* name;
	
	int type;
	union {
		float* floats;
		int64_t* ints;
	};
	size_t count;
	
	int format;
	
	
} ColladaSource;


typedef struct {
	
	char* id;
	char* name;
	
	HashTable sources;
	
} ColladaMesh;


typedef struct {
	char* id;
	
	int type; // 0 = f, 1 = d, 2 = int
	union {
		float* f;
		double* d;
		int64_t* i64;
	} data;
	
	int count;
	int stride;
	
	char access[4];
	
	
} ColladaGeomSource;

typedef struct {
	char* name;
	char* id;
	
	
	HashTable meshes;
	
} ColladaGeometry;





typedef struct {
// 	HashTable meshes;
	char upAxis;
	
	// for loading
	HashTable geometries;
	
	
} ColladaFile;



void colladaLoadFile(char* path);






#endif // COLLADA_H_INCLUDED
