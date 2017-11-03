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


struct { char* name; int len; } propTable[] = {
	[PT_FLOAT] = {"float", 4},
	[PT_DOUBLE] = {"double", 8},
	[PT_INT8] = {"char", 1},
	[PT_INT16] = {"short", 2},
	[PT_INT32] = {"int", 4},
	[PT_INT64] = {"long", 8},
	[PT_UINT8] = {"uchar", 1},
	[PT_UINT16] = {"ushort", 2},
	[PT_UINT32] = {"uint", 4},
	[PT_UINT64] = {"ulong", 8},
};
	

static enum PropType parsePropType(char** s) {
	int i;
	
	while(i = 0; i < PT_MAX_VAL; i++) {
		if(!checkstr(*s, propTable[i].name)) continue;
		
		*s += strlen(propTable[i].name);
		return i;
	}
	
	return PT_NONE;
}

// returns true if there's more
static int parseHeader(PLYContents* pc, char** s) {

	enum PropType t;
	ply_elem* e;
	
	
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
			
			{
				char* end = strchr(*s, ' ');
				e->name = strndup(*s, end - *s);
				*s = end + 1;
			}
			
			e->count = strtol(*s, NULL, 10);
			
			if(checkstr(e->name, "vertex")) {
				pc->numVertices = e->count;
			}
			else if(checkstr(e->name, "face")) {
				pc->numFaces = e->count;
			}
			
			e->stride = 0;
			
			skipline(s);
		}
		else if(checkstr(*s, "property")) {
			t = parsePropType(*s);
			*s++;
			
			if(*(s+1) == ' ') {
				// TODO: make sure we're in the right element
				switch(**s) {
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
			
			e->stride += propTable[t].len;
			
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
	
	
	for(i = 0; i < VEC_LEN(&pc->elements); i++) {
		ply_elem* e = VEC_ITEM(&pc->elements, i);
		
		if(!strcmp(e->name, "vertex")) {
			parseVertices(pc, s);
		}
		else if(!strcmp(e->name, "face")) {
			parseFaces(pc, s);
		}
		
		
	}
	
	
	
	
	return pc;
	
FAIL:
	destroyPLYContents(pc);
	free(pc);
	
	return NULL;
}















