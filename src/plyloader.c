#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "plyloader.h"


#define checkstr(a, b) (0 == strncmp(a, b, strlen(b)))


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
	
	
	while(1) { //printf("-looping: %.16s\n", *s);
		
		if(checkstr(*s, "comment")) { 
		//	printf("found comment\n");
			(*s)++;
			
			skipline(s);
		}
		else if(checkstr(*s, "obj_info")) { 
		//	printf("found obj_info\n");
			(*s)++;
			
			skipline(s);
		}
		else if(checkstr(*s, "format")) {
		//	printf("found format\n");
			(*s)++;
			
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
			printf("elem %s: %d\n", e->name, e->count);
			
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
				// TODO save name
				printf("--propname: %.5s\n", *s);
				p->name = dupname(s);
				
// 				if(p->name[1] == NULL) {
// 					// TODO: make sure we're in the right element
// 					switch(p->name[0]) {
// 						case 'r': pc->r_offset = e->stride; break;;
// 						case 'g': pc->g_offset = e->stride; break;;
// 						case 'b': pc->b_offset = e->stride; break;;
// 						
// 						case 'x': pc->x_offset = e->stride; break;;
// 						case 'y': pc->y_offset = e->stride; break;;
// 						case 'z': pc->z_offset = e->stride; break;;
// 						
// 						case 'u': pc->u_offset = e->stride; break;;
// 						case 'v': pc->v_offset = e->stride; break;;
// 					}
// 				}
				
				e->stride += propSize[t];
			}
			else {
				p->list_len = parsePropType(s);
				(*s)++;
				p->list_item = parsePropType(s);
				(*s)++;
				
				printf("--list types: %d %d \n", p->list_len, p->list_item);
				
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
		else {
			// huh?
			fprintf(stderr, "ERROR: unknown line found in ply header: %.10s\n", *s);
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
	char* contents;
	PLYContents* pc;
	
	contents = readFile(path, &len);
	
	pc = PLYContents_load(contents, len);
	
	free(contents);
	
	return pc;
}


PLYContents* PLYContents_load(char* contents, size_t length) {
	int i, j;
	PLYContents* pc;
	char* s = contents;
	
	// read header
	if(!strncpy(s, "ply", strlen("ply"))) {
		fprintf(stderr, "Missing ply magic string.\n");
		return NULL;
	}
	
	pc = allocPLYContents();
	
	printf("elemlen %d\n", VEC_LEN(&pc->elements));
	
	// grab data in the order specified
	if(parseHeader(pc, &s)) {
		fprintf(stderr, "PLY header parsing failed\n");
		goto FAIL;
	}
	printf("elemlen %d\n", VEC_LEN(&pc->elements));
	
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
	
	
	// only supports LE binary files atm
	for(i = 0; i < VEC_LEN(&pc->elements); i++) {
		ply_elem* e = &VEC_ITEM(&pc->elements, i);
		
		if(!strcmp(e->name, "vertex")) {
			parseVertices(pc, e, &s);
		}
		else if(!strcmp(e->name, "face")) {
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
	
	
	
	
	return pc;
	
FAIL:
	destroyPLYContents(pc);
	free(pc);
	
	return NULL;
}


static float readValueF(enum PropType t, char** s) {
	switch(t) {
		case PT_FLOAT: // wtf?
			return *((float*)(*s));
			
		case PT_DOUBLE: // wtf?
			return *((double*)(*s));
			
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
			*s += propSize[p->type];
		}
		
	}
}


static void skipProp(ply_prop* p, char** s) {
	
	if(p->type != PT_LIST) {
		s += propSize[p->type];
		return;
	}
	
	int len = readValueI(p->list_len, s);
	s += propSize[p->type] * len;
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
	
	
	
	for(i = 0; i < e->count; i++) {
		Vector* t;
		
		VEC_INC(&pc->vertices);
		t = &VEC_TAIL(&pc->vertices);
		
		
		for(j = 0; j < VEC_LEN(&e->props); j++) {
			int index;
			ply_prop* p = &VEC_ITEM(&e->props, j);
			
			// large faces are triangulated
			if(j == indicesIndex) {
				int len = readValueI(p->list_len, s);
				
				int a = readValueI(p->list_item, s);
				int b = readValueI(p->list_item, s);
				
				for(int k = 2; k < len; k++) {
					int c = readValueI(p->list_item, s);
					
					VEC_PUSH(&pc->faces, a);
					VEC_PUSH(&pc->faces, b);
					VEC_PUSH(&pc->faces, c);
					
					b = c;
				}
				
			} 
			else if(j == texIndex) {
				
				// HACK TODO: fix
				skipProp(p, s);
			}
			else {
				skipProp(p, s);
			}
			
		}
	}
	
}













