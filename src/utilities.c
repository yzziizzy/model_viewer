
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include <dirent.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>


#include "utilities.h"



void _glexit(char* msg, const char* file, int line, const char* func) {
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		fprintf(stderr, "GL ERROR at %s:%d (%s): %s: %s \n", file, line, func, msg, gluErrorString(err));
		exit(-1);
	}
}


char* _glerr(char* msg, const char* file, int line, const char* func) {
	char* errstr;
	GLenum err;
	
	err = glGetError();
	errstr = NULL;
	
	if (err != GL_NO_ERROR) {
		errstr = (char*)gluErrorString(err);
#ifndef NO_GL_GET_ERR_DEBUG
		fprintf(
			stderr,
				TERM_BOLD TERM_COLOR_RED "GL ERROR:" TERM_RESET TERM_COLOR_RED
				"GL ERROR at %s:%d (%s): %s: %s \n",
			file, line, func, msg, errstr);
#endif
	}
	
	return errstr;
}


void _khr_debug_callback( // i hate this stype of formatting, but this function has too many damn arguments
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar *message,
	GLvoid *userParam) {

	printf(TERM_BOLD TERM_COLOR_RED "GL ERROR:" TERM_RESET TERM_COLOR_RED " %s\n" TERM_RESET, message);
	
}

void initKHRDebug() {
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, NULL, GL_FALSE);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
	
	glDebugMessageCallback(_khr_debug_callback , NULL);
	

}

char* readFile(char* path, int* srcLen) {
	
	int fsize;
	char* contents;
	FILE* f;
	
	
	f = fopen(path, "rb");
	if(!f) {
		fprintf(stderr, "Could not open file \"%s\"\n", path);
		return NULL;
	}
	
	fseek(f, 0, SEEK_END);
	fsize = ftell(f);
	rewind(f);
	
	contents = (char*)malloc(fsize + 2);
	
	fread(contents+1, sizeof(char), fsize, f);
	contents[0] = '\n';
	contents[fsize] = 0;
	
	fclose(f);
	
	if(srcLen) *srcLen = fsize + 1;
	
	return contents;
}

int getVAOItemSize(VAOConfig* item) {
	switch(item->type) {
		case GL_FLOAT: 
			return 4 * item->sz; 
		case GL_SHORT: 
		case GL_UNSIGNED_SHORT: 
			return 2 * item->sz; 
		case GL_BYTE: 
		case GL_UNSIGNED_BYTE: 
			return 1 * item->sz; 
		default:
			fprintf(stderr, "Unsupported VAO type\n");
			exit(2);
	}
}

int getVAOStride(VAOConfig* details) {
	int i; // packed data is expected
	int stride = 0;

	for(i = 0; details[i].sz != 0; i++) {
		int type_size = 0;
		switch(details[i].type) {
			case GL_FLOAT: 
				type_size = 4; 
				break;
			case GL_SHORT: 
			case GL_UNSIGNED_SHORT: 
				type_size = 2; 
				break;
			case GL_BYTE: 
			case GL_UNSIGNED_BYTE: 
				type_size = 1; 
				break;
			default:
				fprintf(stderr, "Unsupported VAO type\n");
				exit(2);
		}
		
		stride += details[i].sz * type_size;
	}
	
	return stride;
}

GLuint makeVAO(VAOConfig* details) {
	int i; // packed data is expected
	uintptr_t offset = 0;
	int stride;
	GLuint vao;
	
	
	stride = getVAOStride(details);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
	for(i = 0; details[i].sz != 0; i++) {
		GLenum t;
		int ds;
		
		glEnableVertexAttribArray(i);
		t = details[i].type;
		if(t == GL_FLOAT) { // works only for my usage
			glVertexAttribFormat(i, details[i].sz, t, GL_FALSE, (void*)offset);
		}
		else if(details[i].normalize) {
			glVertexAttribFormat(i, details[i].sz, t, GL_TRUE, (void*)offset);
			glexit("");
		}
		else {
			glVertexAttribIFormat(i, details[i].sz, t, (void*)offset);
		}
		glerr("vao init");
		
		if(t == GL_UNSIGNED_BYTE || t == GL_BYTE) ds = 1;
		else if(t == GL_UNSIGNED_SHORT || t == GL_SHORT) ds = 2;
		else ds = 4;
		
		offset += ds * details[i].sz;
	}
	glexit("vao init");
	
	return vao;
}



// length of the line, or length of the string if no \n found
size_t strlnlen(const char* s) {
	char* n;
	
	n = strchr(s, '\n');
	if(!n) return strlen(s);
	
	return n - s;
}

// strdup a line
char* strlndup(const char* s) {
	return strndup(s, strlnlen(s));
}

// line count;
int strlinecnt(const char* s) {
	int n;

	if(!*s) return 0;
	
	n = 1;
	while(*s) // just to make you cringe
		if(*s++ == '\n') 
			n++;
	
	return n;
}

char* pathJoin(const char* a, const char* b) {
	int alen, blen;
	char* o;
	
	
	alen = a ? strlen(a) : 0;
	blen = b ? strlen(b) : 0;
	
	o = malloc(alen + blen + 2);
	
	strcpy(o, a ? a : "");
	o[alen] = '/'; // TODO: fix the concat here
	strcpy(o + alen + 1, b ? b : "");
	o[alen + blen + 1] = 0; 
	
	return o;
}


void skipWhitespace(char** s) {
	char* e;
	e = *s + strspn(*s, " \n\t\r\v");
	
	if(e) {
		*s = e;
		return;
	}
	
	// no whitespace until the end of the string
	*s = *s + strlen(*s);
}

