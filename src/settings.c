

 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "c3dlas/c3dlas.h"
#include "c3dlas/meshgen.h"
#include "text/text.h"
#include "c_json/json.h"
#include "json_gl.h"


#include "utilities.h"
#include "config.h"
#include "shader.h"
#include "window.h"
#include "game.h"



/*

normals on a mesh

bounding box rendering

axes rendering

adjacency list

line strip rendering
point list rendering


math functions for various views
	left/right, up vector, look at point from distance



*/


void loadSettings(char* path, GameState* gs) {
	
	json_file_t* jsf = json_load_path("assets/config/settings.json");
	
	json_value_t* tex;
	json_obj_get_key(jsf->root, "kb", &tex);
	
	
	json_obj_unpack_struct(tex,
		JSON_UNPACK(&gs->uSettings, keyRotateSensitivity, JSON_TYPE_FLOAT),
		JSON_UNPACK(&gs->uSettings, keyScrollSensitivity, JSON_TYPE_FLOAT),
		JSON_UNPACK(&gs->uSettings, keyZoomSensitivity, JSON_TYPE_FLOAT),
		   
		JSON_UNPACK(&gs->uSettings, mouseRotateSensitivity, JSON_TYPE_FLOAT),
		JSON_UNPACK(&gs->uSettings, mouseScrollSensitivity, JSON_TYPE_FLOAT),
		JSON_UNPACK(&gs->uSettings, mouseZoomSensitivity, JSON_TYPE_FLOAT),
		NULL
	);
	
}













