

bin_PROGRAMS = glmv

glmv_SOURCES = main.c \
	hash.c \
	MurmurHash3.c \
	dumpImage.c \
	utilities.c \
	settings.c \
	objloader.c \
	staticMesh.c \
	uniformBuffer.c \
	timing.c \
	fbo.c \
	window.c \
	game.c \
	shader.c \
	texture.c \
	json_gl.c \
	c_json/json.c \
	c3dlas/c3dlas.c \
	c3dlas/meshgen.c \
	text/text.c \
	text/fcfg.c

glmv_LDADD =  \
	-lm \
	-lX11 -lfreetype -lfontconfig -lpng \
	-lGL -lGLU -lGLEW
	 
AM_CFLAGS = "-I/usr/include/freetype2" -g -DLINUX -Wno-unused-result -fdiagnostics-color
AM_CXXFLAGS = $(AM_CFLAGS)



