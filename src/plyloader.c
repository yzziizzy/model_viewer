#include <stdlibh>
#include <stdio.h>
#include <string.h>


#include "plyloader.h"


#define checkstr(a, b) 0 == strncmp(a, b, strlen(b))


PLYContents* allocPLYContents() {
	PLYContents* pc;
	
	pc = calloc(1, sizeof(*pc));
	CHECK_OOM(pc);
	
	VEC_INIT(&pc->comments);
	VEC_INIT(&pc->vertices);
	VEC_INIT(&pc->colors);
	VEC_INIT(&pc->normals);
	VEC_INIT(&pc->texcoords);
	VEC_INIT(&pc->faces);
	VEC_INIT(&pc->elements);
	
	return pc;
}

void destroyPLYContents(PLYContents* pc) {
	
	VEC_FREE(&pc->comments);
	VEC_FREE(&pc->vertices);
	VEC_FREE(&pc->colors);
	VEC_FREE(&pc->normals);
	VEC_FREE(&pc->texcoords);
	VEC_FREE(&pc->faces);
	VEC_FREE(&pc->elements);
}


static void skipline(char** s) {
	while(**s && **s != '\n') (*s)++;
	(*s)++;
}

static char* dupname(char** s) {
	char* n;
	char* end = strchr(*s, ' ');
	n = strndup(*s, end - *s);
	
	*s = end + 1;
	
	return n;
}


struct { char* name; int len; } propSize[] = {
	[PT_FLOAT] = {4},
	[PT_DOUBLE] = {8},
	[PT_INT8] = {1},
	[PT_INT16] = {2},
	[PT_INT32] = {4},
	[PT_INT64] = {8},
	[PT_UINT8] = {1},
	[PT_UINT16] = {2},
	[PT_UINT32] = {4},
	[PT_UINT64] = {8},
};
	
struct { enum PropType type; char* name; } propTable[] = {
	// standard names
	{PT_FLOAT, "float"},
	{PT_DOUBLE, "double"},
	{PT_INT8, "char"},
	{PT_INT16, "short"},
	{PT_INT32, "int"},
	{PT_INT64, "long"},
	{PT_UINT8, "uchar"},
	{PT_UINT16, "ushort"},
	{PT_UINT32, "uint"},
	{PT_UINT64, "ulong"},
	
	// colmap uses this style
	{PT_FLOAT, "float32"},
	{PT_DOUBLE, "double64"},
	{PT_INT8, "int8"},
	{PT_INT16, "int16"},
	{PT_INT32, "int32"},
	{PT_INT64, "int64"},
	{PT_UINT8, "uint8"},
	{PT_UINT16, "uint16"},
	{PT_UINT32, "uint32"},
	{PT_UINT64, "uint64"},
};
	

static enum PropType parsePropType(char** s) {
	int i;
	int len = sizeof(propTable) / sizeof(propTable[0]);
	
	while(i = 0; i < len; i++) {
		if(!checkstr(*s, propTable[i].name)) continue;
		
		*s += strlen(propTable[i].name);
		return propTable[i].type;
	}
	
	return PT_MAX_VAL;
}



// returns true if there's more
static int parseHeader(PLYContents* pc, char** s) {
	
	enum PropType t, list_len_type, list_type;
	ply_elem* e;
	ply_prop* p;
	
	
	while(1) {
		
		if(checkstr(*s, "comment")) {
			*s++;
			
			skipline(s);
		}
		else if(checkstr(*s, "format")) {
			*s++;
			
			if(checkstr(*s, "ascii")) {
				pc->isBinary = 0;
				pc->isBigEndian = 0;
			}
			else if(checkstr(*s, "binary_little_endian")) {
				pc->isBinary = 1;
				pc->isBigEndian = 0;
			}
			else if(checkstr(*s, "binary_big_endian")) {
				pc->isBinary = 1;
				pc->isBigEndian = 1;
				
				fprintf(stderr, "WARNING: big-endian ply files are not support. please upgrade your files to the 21st century.\n");
			}
			
			skipline(s);
		}
		else if(checkstr(*s, "element")) {
			// 'element' lines contain how many are in the file and start the property list  
			*s++;
			
			VEC_INC(&pc->elements);
			e = &VEC_TAIL(&pc->elements);
			VEC_INIT(&e->props);
			
			e->name = dupname(*s);
			e->count = strtol(*s, NULL, 10);
			
			if(checkstr(e->name, "vertex")) {
				pc->numVertices = e->count;
				pc->foundVertices = 1;
			}
			else if(checkstr(e->name, "face")) {
				pc->numFaces = e->count;
				pc->foundFaces = 1;
			}
			
			e->stride = 0;
			
			skipline(s);
		}
		else if(checkstr(*s, "property")) {
			VEC_INC(&e->props);
			p = &VEC_TAIL(&e->props);
			
			p->type = t = parsePropType(*s);
			*s++;
			
			if(t == PT_LIST) {
				// TODO save name
				p->name = dupname(*s);

				
				if(p->name[1] == NULL) {
					// TODO: make sure we're in the right element
					switch(p->name[0]) {
						case 'r': pc->r_offset = e->stride; break;;
						case 'g': pc->g_offset = e->stride; break;;
						case 'b': pc->b_offset = e->stride; break;;
						
						case 'x': pc->x_offset = e->stride; break;;
						case 'y': pc->y_offset = e->stride; break;;
						case 'z': pc->z_offset = e->stride; break;;
						
						case 'u': pc->u_offset = e->stride; break;;
						case 'v': pc->v_offset = e->stride; break;;
					}
				}
				
				e->stride += propSize[t];
			}
			else {
				p->list_len = parsePropType(*s);
				*s++;
				p->list_item = parsePropType(*s);
				*s++;
				
				p->name = dupname(*s);
				
				// check for special names 
				if(checkstr(e->name, "vertex_indices")) {
					pc->foundIndices = 1;
				}
				else if(checkstr(e->name, "texcood")) {
					pc->foundFaceTexCoords = 1;
				}
			}
			
			skipline(s);
		}
		else if(checkstr(*s, "ply")) {
			skipline(s);
		}
		else if(checkstr(*s, "end_header")) {
			skipline(s);
			
			return 0;
		}
		else {
			// huh?
			fprintf(stderr, "ERROR: unknown line found in ply header\n;");
			return 1;
		}
	
	}
	
}




PLYContents* PLYContents_loadPath(char* path) {
	size_t len;
	char* contents;
	PLYContents* pc;
	
	contents = readFile(path, &len);
	
	pc = PLYContents_load(contents, len);
	
	free(contents);
	
	return pc;
}


PLYContents* PLYContents_load(char* contents, size_t length) {
	int i;
	PLYContents* pc;
	char* s = contents;
	
	// read header
	if(!strncpy(s, "ply", strlen("ply"))) {
		fprintf(stderr, "Missing ply magic string.\n");
		return NULL;
	}
	
	pc = allocPLYContents();
	
	
	
	// grab data in the order specified
	if(parseHeader(pc, &s)) {
		fprintf(stderr, "PLY header parsing failed\n");
		goto FAIL;
	}
	
	// print out some stats
	for(i = 0; i < VEC_LEN(&pc->elements); i++) {
		ply_elem* e = &VEC_ITEM(pc->elements, i);
		printf("element %d: %s [%d]\n", i, e->name, e->count); 
		printf("  --> stride: %d\n", e->stride);
		
		for(j = 0; j < VEC_LEN(&e->props); j++) {
			ply_prop* p = &VEC_ITEM(&e->props);
			
			printf("  prop %d: %s\n", j, p->name);
		}
		
	}
	
	
	for(i = 0; i < VEC_LEN(&pc->elements); i++) {
		ply_elem* e = VEC_ITEM(&pc->elements, i);
		
		if(!strcmp(e->name, "vertex")) {
			parseVertices(pc, s);
		}
		else if(!strcmp(e->name, "face")) {
			parseFaces(pc, s);
		}
		
		// temporary solution:
		//  record original vertices size (numVertices should be it) 
		// store first found tex coord in texcoords
		// every new one initiates search starting from numVertices
		// if a complete vertex match is not found, a new vertex is created
	}
	
	
	
	
	return pc;
	
FAIL:
	destroyPLYContents(pc);
	free(pc);
	
	return NULL;
}



static void parseVertices(PLYContents* pc, char** s) {
	
	
	
}

static void parseFaces(PLYContents* pc, char** s) {
	
	
	
}













