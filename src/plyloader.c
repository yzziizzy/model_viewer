#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>


#include "plyloader.h"


#define checkstr(a, b) (0 == strncmp(a, b, strlen(b)))
#define checkstrcase(a, b) (0 == strncasecmp(a, b, strlen(b)))


static void parseVertices(PLYContents* pc, ply_elem* e, char** s);
static void parseFaces(PLYContents* pc, ply_elem* e, char** s);



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
	VEC_INIT(&pc->indicesTex);
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
	//printf("skipping: %.10s\n", *s);
	while(**s && **s != '\n') (*s)++;
	(*s)++;
}

static char* dupname(char** s) {
	char* n;
	char* end = strpbrk(*s, " \n\r");
	n = strndup(*s, end - *s);
	
	*s = end + 1;
	
	return n;
}


int propSize[] = {
	[PT_FLOAT] = 4,
	[PT_DOUBLE] = 8,
	[PT_INT8] = 1,
	[PT_INT16] = 2,
	[PT_INT32] = 4,
	[PT_INT64] = 8,
	[PT_UINT8] = 1,
	[PT_UINT16] = 2,
	[PT_UINT32] = 4,
	[PT_UINT64] = 8,
};
	
struct { enum PropType type; char* name; } propTable[] = {
	// standard names
	{PT_LIST, "list"},
	
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
	
	for(i = 0; i < len; i++) {
		if(!checkstr(*s, propTable[i].name)) continue;
		if(*(*s + strlen(propTable[i].name)) != ' ') continue;
		
		*s += strlen(propTable[i].name);
		return propTable[i].type;
	}
	
	return PT_MAX_VAL;
}



char* realPathFromSiblingPath(char* sibling, char* file) {
	char* falsePath, *realPath;
	
	char* fuckdirname = strdup(sibling);
	char* dir;
	
	dir = dirname(fuckdirname);
	
	falsePath = pathJoin(dir, file);
	
	realPath = realpath(falsePath, NULL);
	if(!realPath) {
		// handle errno
	}
	
	free(fuckdirname);
	free(falsePath);
	
	return realPath;
}


// returns true if there's more
static int parseHeader(PLYContents* pc, char** s) {
	
	enum PropType t, list_len_type, list_type;
	ply_elem* e;
	ply_prop* p;
	
	
	while(1) { 
		
		if(checkstr(*s, "comment")) { 
			*s += strlen("comment") + 1;
			char* save_ss = *s;
			//printf("comment line: '%.20s'\n", *s);
			if(checkstrcase(*s, "texturefile")) {
				*s += strlen("texturefile") + 1;
				char* ts = dupname(s);
				pc->texPath = realPathFromSiblingPath(pc->path, ts);
				free(ts);
			}
			*s = save_ss;
			
			skipline(s);
		}
		else if(checkstr(*s, "obj_info")) { 
			(*s)++;
			
			skipline(s);
		}
		else if(checkstr(*s, "format")) {
			*s += strlen("format") + 1;
			
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
		//	printf("%.10s", *s);
		}
		else if(checkstr(*s, "element")) {
			// 'element' lines contain how many are in the file and start the property list  
			(*s) += strlen("element") + 1;
			VEC_INC(&pc->elements);
			e = &VEC_TAIL(&pc->elements);
			VEC_INIT(&e->props);
			
			
			e->name = dupname(s);
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
			
			(*s) += strlen("property") + 1;
			
			p->type = t = parsePropType(s);
			(*s)++;
			
			if(t != PT_LIST) {
				p->name = dupname(s);
				printf("pname: %.20s\n", p->name);
				e->stride += propSize[t];
			}
			else {
				p->list_len = parsePropType(s);
				(*s)++;
				p->list_item = parsePropType(s);
				(*s)++;
				
				p->name = dupname(s);
				e->hasLists = 1;
				
				// check for special names 
				if(checkstr(e->name, "vertex_indices")) {
					pc->foundIndices = 1;
				}
				else if(checkstr(e->name, "texcood")) {
					pc->foundFaceTexCoords = 1;
				}
			}
			
		//	printf("prop %s [%d]\n", p->name, p->type);
			
			//skipline(s);
		}
		else if(checkstr(*s, "ply")) {
		//	printf("found ply\n");
			skipline(s);
		}
		else if(checkstr(*s, "end_header")) {
			skipline(s);
			
			return 0;
		}
		else if(**s == '\n' || **s == '\r' || **s == ' ' || **s == '\t') {
			skipline(s); //(*s)++;
		}
		else {
			// huh?
			fprintf(stderr, "ERROR: unknown line found in ply header: %.10s\n", *s);
			*((char*)0) = 1;
			exit(1);
			skipline(s);
			
		}
	
	}
	
}


int64_t readValueI(enum PropType t, char** s) {
	int64_t l = 0;
	
	switch(t) {
		case PT_INT8:
		case PT_UINT8:
			l = **s;
			*s += 1;
			break;
			
		case PT_INT16:
		case PT_UINT16:
			l = *((int16_t*)(*s));
			*s += 2;
			break;
			
		case PT_INT32:
		case PT_UINT32:
			l = *((int32_t*)(*s));
			*s += 4;
			break;
			
		case PT_INT64:
		case PT_UINT64:
			l = *((int64_t*)(*s));
			*s += 8;
			break;
			
		case PT_FLOAT: // wtf?
			l = *((float*)(*s));
			*s += 4;
			break;
			
		case PT_DOUBLE: // wtf?
			l = *((double*)(*s));
			*s += 8;
			break;
	}
	
	return l;
}



PLYContents* PLYContents_loadPath(char* path) {
	size_t len;
	char* contents, *s;
	PLYContents* pc;
	int i, j;
	
	contents = readFile(path, &len);
	s = contents;
	
	
	pc = allocPLYContents();
	pc->path = path;
	
	// grab data in the order specified
	if(parseHeader(pc, &s)) {
		fprintf(stderr, "PLY header parsing failed\n");
		goto FAIL;
	}
	
	// print out some stats
	for(i = 0; i < VEC_LEN(&pc->elements); i++) {
		ply_elem* e = &VEC_ITEM(&pc->elements, i);
		printf("element %d: %s [%d] {%d}\n", i, e->name, e->count, VEC_LEN(&e->props)); 
		//printf("  --> stride: %d\n", e->stride);
		
		for(j = 0; j < VEC_LEN(&e->props); j++) {
			ply_prop* p = &VEC_ITEM(&e->props, j);
			
			printf("  prop %d: %s\n", j, p->name);
		}
		
	}
	printf("vertices %d\n", VEC_LEN(&pc->vertices));
	printf("faces %d\n", VEC_LEN(&pc->faces));
	
	// only supports LE binary files atm
	for(i = 0; i < VEC_LEN(&pc->elements); i++) {
		ply_elem* e = &VEC_ITEM(&pc->elements, i);
		
		if(!strcmp(e->name, "vertex")) {
			printf("-- parsing vertices\n");
			parseVertices(pc, e, &s);
		}
		else if(!strcmp(e->name, "face")) {
			printf("-- parsing faces\n");
			parseFaces(pc, e, &s);
		}
		else { // unrecognized elements are skipped
			if(!e->hasLists) {
				// fixed size element sets can be skipped with math
				s += e->stride * e->count;
			}
			else {
				// elements with lists have to be parsed individually in binary files
				for(int j = 0; j < VEC_LEN(&e->props); j++) {
					ply_prop* p = &VEC_ITEM(&e->props, j);
					
					if(p->type != PT_LIST) {
						s += propSize[p->type];
						continue;
					}
					
					int len = readValueI(p->list_len, &s);
					
					s += propSize[p->type] * len;
				}
			}
			
		}
		
		// temporary solution:
		//  record original vertices size (numVertices should be it) 
		// store first found tex coord in texcoords
		// every new one initiates search starting from numVertices
		// if a complete vertex match is not found, a new vertex is created
	}
	
	printf("vertices %d\n", VEC_LEN(&pc->vertices));
	printf("faces %d\n", VEC_LEN(&pc->faces));
	free(contents);
	
	
	return pc;
	
FAIL:
	destroyPLYContents(pc);
	free(pc);
	free(contents);
	
	return NULL;
}


static double readValueF(enum PropType t, char** s) {
	double f;
	switch(t) {
		case PT_FLOAT: // wtf?
			f = *((float*)(*s));
			*s += sizeof(float);
			return f;
			
		case PT_DOUBLE: // wtf?
			f = *((double*)(*s));
			*s += sizeof(double);
			return f;
			
		default:
			fprintf(stderr, "PLY Loader: tried to read a non-float value.\n");
			return 0;
	}
}


static void parseVertices(PLYContents* pc, ply_elem* e, char** s) {
	int i, j;
	
	for(i = 0; i < e->count; i++) {
		Vector* t;
		
		VEC_INC(&pc->vertices);
		t = &VEC_TAIL(&pc->vertices);
		
		
		for(int j = 0; j < VEC_LEN(&e->props); j++) {
			float f;
			ply_prop* p = &VEC_ITEM(&e->props, j);
			
			
			if(p->name[1] == '\0') {
				f = readValueF(p->type, s);
				
				switch(p->name[0]) {
					case 'x': case 'X': t->x = f; break;;
					case 'y': case 'Y': t->y = f; break;;
					case 'z': case 'Z': t->z = f; break;;
				}
			}
			
			// TODO: handle list types
			//*s += propSize[p->type];
		}
		
		//printf("found vertex: [%.2f, %.2f, %.2f]\n", t->x, t->y, t->z);
		
	}
}


static void skipProp(ply_prop* p, char** s) {
	
	if(p->type != PT_LIST) {
		s += propSize[p->type];
		return;
	}
	
	int len = readValueI(p->list_len, s);
// 	printf("proplen: %d * %d ", len, propSize[p->list_item]);
// 		printf(" {%ul} ", (uint64_t)*s);
				
	*s += propSize[p->list_item] * len;
// 		printf(" {%ul} \n", (uint64_t)*s);
				
}


static void parseFaces(PLYContents* pc, ply_elem* e, char** s) {
	int i, j;
	short indicesIndex = -1;
	short texIndex = -1;
	
	
	// cache property indices for things we want to avoid strcmp in the inner loop
	for(j = 0; j < VEC_LEN(&e->props); j++) {
		ply_prop* p = &VEC_ITEM(&e->props, j);
		
		if(0 == strcmp(p->name, "vertex_indices")) { // used by openmvs
			indicesIndex = j;
		}
		else if(0 == strcmp(p->name, "texcoord")) { // used by openmvs
			texIndex = j;
		}
	}
	
	
	uint64_t oldpos, newpos;
	oldpos = *s;
	float minu = 1, maxu = 0, minv = 1, maxv = 0;
	
	for(i = 0; i < e->count; i++) {
		Vector* t;
		
		
		for(j = 0; j < VEC_LEN(&e->props); j++) {
			int index;
			ply_prop* p = &VEC_ITEM(&e->props, j);
			
			// large faces are triangulated
			if(j == indicesIndex) {
				int len = readValueI(p->list_len, s);
				
			//		printf(" z{%ul} ", (uint64_t)*s);
				int a = readValueI(p->list_item, s);
// 					printf(" a{%ul} ", (uint64_t)*s);
				int b = readValueI(p->list_item, s);
// 					printf(" b{%ul} ", (uint64_t)*s);
				
// 				printf("len: %d [%d, %d", len, a, b);
				
				for(int k = 2; k < len; k++) {
					int c = readValueI(p->list_item, s);
// 					printf(" {%ul} ", (uint64_t)*s);
// 					printf(", %d");
					VEC_PUSH(&pc->faces, a);
					VEC_PUSH(&pc->faces, b);
					VEC_PUSH(&pc->faces, c);
					
					b = c;
				}
				
// 				printf("]\n");
			} 
			else if(j == texIndex) {
// 				printf("texcoord\n");
				// HACK TODO: fix
// 			printf(" {%ul} ", (uint64_t)*s);
				char* s_save = *s;
				
				int len = readValueI(p->list_len, s);
				//printf("tex len: %d\n", len);
				
				if(len != 6) {
					printf("unsupported texcoord length of %d found; skipping data (will ruin model)\n", len);
				}
				
				float fa, fb;
				
				
				// hope you have GCC...
				void checkTexBounds(float u, float v) {
					if(u > 1 || u < 0) printf("texture U coordinate out of bounds: %.7f\n", u);
					if(v > 1 || v < 0) printf("texture V coordinate out of bounds: %.7f\n", v);
				//printf("[%.9f, %.9f]\n", u, v);
					minu = fmin(u, minu);
					minv = fmin(v, minv);
					maxu = fmax(u, maxu);
					maxv = fmax(v, maxv);
				}
				
				// 1
				fa = readValueF(p->list_item, s);
				fb = readValueF(p->list_item, s);
			//	printf("tc: %.5f, %.5f\n", fa, fb);
				
				checkTexBounds(fa, fb);
				VEC_INC(&pc->texcoords);
				VEC_TAIL(&pc->texcoords).x = fa;
				VEC_TAIL(&pc->texcoords).y = fb;
				
				VEC_PUSH(&pc->indicesTex, VEC_LEN(&pc->texcoords) - 1);
				
				
				// 2
				fa = readValueF(p->list_item, s);
				fb = readValueF(p->list_item, s);
			//	printf("tc: %.5f, %.5f\n", fa, fb);
				checkTexBounds(fa, fb);
				VEC_INC(&pc->texcoords);
				VEC_TAIL(&pc->texcoords).x = fa;
				VEC_TAIL(&pc->texcoords).y = fb;
				
				VEC_PUSH(&pc->indicesTex, VEC_LEN(&pc->texcoords) - 1);
				
				
				// 3
				fa = readValueF(p->list_item, s);
				fb = readValueF(p->list_item, s);
			//	printf("tc: %.5f, %.5f\n", fa, fb);
				checkTexBounds(fa, fb);
				VEC_INC(&pc->texcoords);
				VEC_TAIL(&pc->texcoords).x = fa;
				VEC_TAIL(&pc->texcoords).y = fb;
				
				VEC_PUSH(&pc->indicesTex, VEC_LEN(&pc->texcoords) - 1);
				
				
				*s = s_save;
				skipProp(p, s); // printf(" {%ul} ", (uint64_t)*s);
			}
			else {
				skipProp(p, s);
			}
			
			newpos = *s;
		
// 			printf("oldpos: %ul, newpos: %ul, dist: %ul \n", oldpos, newpos, newpos - oldpos);
			oldpos = newpos;
			
			
		}
		
		

	}
	

	
}













