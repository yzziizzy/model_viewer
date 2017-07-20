
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <unistd.h>


#include <X11/X.h>
#include <X11/Xlib.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "c3dlas/c3dlas.h"
#include "c3dlas/meshgen.h"
#include "c_json/json.h"
#include "json_gl.h"

#include "utilities.h"
#include "config.h"
#include "objloader.h"
#include "shader.h"
#include "texture.h"
#include "window.h"
#include "gui.h"
#include "staticMesh.h"
#include "renderable.h"
#include "game.h"
// #include "scene.h"

#include "axes.h"


GLuint proj_ul, view_ul, model_ul;

Matrix mProj, mView, mModel;

float zoom;


// temp shit
GUIText* gt;

StaticMesh* testmesh;
Renderable* testrenderable;

// MapBlock* map;
// TerrainBlock* terrain;

GLuint fsQuadVAO, fsQuadVBO;
ShaderProgram* shadingProg;

void initFSQuad() {
	float vertices[] = {
		-1.0, -1.0, 0.0,
		-1.0, 1.0, 0.0,
		1.0, -1.0, 0.0,
		1.0, 1.0, 0.0
	};

	glGenVertexArrays(1, &fsQuadVAO);
	glBindVertexArray(fsQuadVAO);
	
	glGenBuffers(1, &fsQuadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, fsQuadVBO);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, 0);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

void drawFSQuad() {
	
	glBindVertexArray(fsQuadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, fsQuadVBO);
	glexit("quad vbo");
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glexit("quad draw");
	
}


#define getPrintGLEnum(e, m) _getPrintGLEnumMin(e, #e, m)

int _getPrintGLEnumMin(GLenum e, char* name, char* message) {
	GLint i;
	
	glGetIntegerv(e, &i);
	printf("%s: %d\n", name, i);
	
	return i;
}

static void unpack_fbo(json_value_t* p, char* key, FBOTexConfig* cfg) {
	char* a, *b, *c;
	json_value_t* o, *v1, *v2, *v3;
	
	json_obj_get_key(p, key, &o);
	
	json_obj_get_key(o, "internalType", &v1); a = v1->v.str;
	json_as_GLenum(v1, &cfg->internalType);
	
	json_obj_get_key(o, "format", &v2); b = v2->v.str;
	json_as_GLenum(v2, &cfg->format);
	
	json_obj_get_key(o, "size", &v3); c = v3->v.str;
	json_as_GLenum(v3, &cfg->size);
	
	printf("fbo cfg from json: %s: %x, %s: %x, %s: %x\n", a, cfg->internalType, b, cfg->format, c, cfg->size);
}


void setupFBOs(GameState* gs, int resized) {
	int ww = gs->screen.wh.x;
	int wh = gs->screen.wh.y;
	
	printf("creating fbos\n");
	
	if(gs->fboTextures) {
		destroyFBOTextures(gs->fboTextures);
		free(gs->fboTextures);
		printf("destryed old fbos\n");
	}
	
	json_file_t* jsf = json_load_path("assets/config/fbo.json");
	
	json_value_t* tex;
	json_obj_get_key(jsf->root, "textures", &tex);
	
	FBOTexConfig texcfg2[6];
	unpack_fbo(tex, "diffuse", &texcfg2[0]);
	unpack_fbo(tex, "normal", &texcfg2[1]);
	unpack_fbo(tex, "selection", &texcfg2[2]);
	unpack_fbo(tex, "lighting", &texcfg2[3]);
	unpack_fbo(tex, "depth", &texcfg2[4]);
	texcfg2[5].internalType = 0;
	texcfg2[5].format = 0;
	texcfg2[5].size = 0;
	
	/*
	FBOTexConfig texcfg[6];
	
	json_obj_unpack_struct(tex, 
		JSON_UNPACK(&texcfg[0], internalType, JSON_TYPE_STRING), 
		JSON_UNPACK(&texcfg[0], internalType, JSON_TYPE_STRING), 
		JSON_UNPACK(&texcfg[0], internalType, JSON_TYPE_STRING), 
		JSON_UNPACK(&texcfg[0], internalType, JSON_TYPE_STRING)
					 
					 
	);
	*/
	
	// backing textures
	FBOTexConfig texcfg[] = {
		{GL_RGB, GL_RGB, GL_UNSIGNED_BYTE},
		{GL_RGB, GL_RGB, GL_UNSIGNED_BYTE},
		{GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE},
		{GL_RGB16F, GL_RGB, GL_HALF_FLOAT},
		{GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT},
		{0,0,0}
	};

	printf("\nfbo cfg from code: %x, %x, %x\n", GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
	printf("fbo cfg from code: %x, %x, %x\n", GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
	printf("fbo cfg from code: %x, %x, %x\n", GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
	printf("fbo cfg from code: %x, %x, %x\n", GL_RGB16F, GL_RGB, GL_HALF_FLOAT);
	printf("fbo cfg from code: %x, %x, %x\n", GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT);

	
	GLuint* texids = initFBOTextures(ww, wh, texcfg2);
	
	gs->diffuseTexBuffer = texids[0];
	gs->normalTexBuffer = texids[1];
	gs->selectionTexBuffer = texids[2];
	gs->lightingTexBuffer = texids[3];
	gs->depthTexBuffer = texids[4];
	
	printf("New Main Depth: %d \n", texids[3]);
	
	// main gbuffer setup
	if(gs->gbuf.fb) { // evil abstraction breaking. meh.
		destroyFBO(&gs->gbuf);
	}
	
	FBOConfig gbufConf[] = {
		{GL_COLOR_ATTACHMENT0, gs->diffuseTexBuffer },
		{GL_COLOR_ATTACHMENT1, gs->normalTexBuffer },
//		{GL_COLOR_ATTACHMENT2, gs->lightingTexBuffer },
		{GL_DEPTH_ATTACHMENT, gs->depthTexBuffer },
		{0,0}
	};
	
	initFBO(&gs->gbuf, gbufConf);
	
	// decal pass framebufer
	FBOConfig decalConf[] = {
		{GL_COLOR_ATTACHMENT0, gs->diffuseTexBuffer },
		{GL_COLOR_ATTACHMENT1, gs->normalTexBuffer },
		{GL_COLOR_ATTACHMENT2, gs->lightingTexBuffer },
		{GL_DEPTH_ATTACHMENT,  gs->depthTexBuffer },
		{0,0}
	};
	// depth buffer is also bound as a texture but disabled for writing
	
	initFBO(&gs->decalbuf, decalConf);
	
	// lighting pass framebufer
	FBOConfig lightingConf[] = {
		{GL_COLOR_ATTACHMENT0, gs->lightingTexBuffer },
		{GL_DEPTH_ATTACHMENT,  gs->depthTexBuffer },
		{0,0}
	};
	// depth buffer is also bound as a texture but disabled for writing
	
	initFBO(&gs->lightingbuf, lightingConf);
	
	// selection pass framebufer
	FBOConfig selectionConf[] = {
// 		{GL_COLOR_ATTACHMENT0, gs->diffuseTexBuffer },
// 		{GL_COLOR_ATTACHMENT1, gs->normalTexBuffer },
		{GL_COLOR_ATTACHMENT0, gs->selectionTexBuffer },
		{GL_DEPTH_ATTACHMENT,  gs->depthTexBuffer },
		{0,0}
	};
	// depth buffer is also bound as a texture but disabled for writing
	
	initFBO(&gs->selectionbuf, selectionConf);
	
	
	// pbo's for selection buffer
	gs->readPBO = -1;
	gs->activePBO = 0;
	
	if(gs->selectionPBOs[0]) {
		glDeleteBuffers(2, gs->selectionPBOs);
		glexit("");
	}
	
	glGenBuffers(2, gs->selectionPBOs);
	glexit("");
	
	glBindBuffer(GL_PIXEL_PACK_BUFFER, gs->selectionPBOs[0]);
	glexit("");
	glBufferData(GL_PIXEL_PACK_BUFFER, ww * wh * 4, NULL, GL_DYNAMIC_READ);
	glexit("");
	glBindBuffer(GL_PIXEL_PACK_BUFFER, gs->selectionPBOs[1]);
	glexit("");
	glBufferData(GL_PIXEL_PACK_BUFFER, ww * wh * 4, NULL, GL_DYNAMIC_READ);
	glexit("");
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	
	if(gs->selectionData) free(gs->selectionData);
	printf("seldata size %d\n", ww * wh * 4);
	gs->selectionData = malloc(ww * wh * 4);
	
	printf("done creating fbos\n");
}

void initGame(XStuff* xs, GameState* gs) {
	int ww, wh;
	
	json_gl_init_lookup();
	
	glerr("left over error on game init");
	
	gs->hasMoved = 1;
	gs->lastSelectionFrame = 0;
	gs->frameCount = 0;
	
	gs->activeTool = 0;
	
	gs->debugMode = 0;
	
	gs->nearClipPlane = 20;
	gs->farClipPlane = 1700;

	
	ww = xs->winAttr.width;
	wh = xs->winAttr.height;
	
	gs->screen.wh.x = (float)ww;
	gs->screen.wh.y = (float)wh;
	
	gs->screen.aspect = gs->screen.wh.x / gs->screen.wh.y;
	gs->screen.resized = 0;
	
	
	printf("w: %d, h: %d\n", ww, wh);
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	
	initUniformBuffers();
	
	uniformBuffer_init(&gs->perViewUB, sizeof(PerViewUniforms));
	uniformBuffer_init(&gs->perFrameUB, sizeof(PerFrameUniforms));
	
	setupFBOs(gs, 0);


	
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// set up the Geometry Buffer

	
	shadingProg = loadCombinedProgram("shading");
	glexit("");
	printf("loaded shading prog\n");
	glUseProgram(shadingProg->id);
	glUniform1i(glGetUniformLocation(shadingProg->id, "sDiffuse"), 0);
	glUniform1i(glGetUniformLocation(shadingProg->id, "sNormals"), 1);
	glUniform1i(glGetUniformLocation(shadingProg->id, "sDepth"), 2);
 	glUniform1i(glGetUniformLocation(shadingProg->id, "sSelection"), 3);
 	glUniform1i(glGetUniformLocation(shadingProg->id, "sLighting"), 4);
	
	glexit("");
	printf("set buffs\n");
	initFSQuad();
	
	// check some prerequisites
// 	GLint MaxDrawBuffers = 0;
// 	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &MaxDrawBuffers);
// 	printf("MAX_DRAW_BUFFERS: %d\n", MaxDrawBuffers);
//  	if(MaxDrawBuffers < 4) {
// 		fprintf(stderr, "FATAL: GL_MAX_DRAW_BUFFERS is too low: %d. Minimum required value is 4.\n", MaxDrawBuffers);
// 		exit(3);
// 	};

//	getPrintGLEnum(GL_MAX_COLOR_ATTACHMENTS, "meh");
	getPrintGLEnum(GL_MAX_DRAW_BUFFERS, "meh");
//	getPrintGLEnum(GL_MAX_FRAMEBUFFER_WIDTH, "meh");
//	getPrintGLEnum(GL_MAX_FRAMEBUFFER_HEIGHT, "meh");
//	getPrintGLEnum(GL_MAX_FRAMEBUFFER_LAYERS, "meh");
//	getPrintGLEnum(GL_MAX_FRAMEBUFFER_SAMPLES, "meh");
	getPrintGLEnum(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, "meh");
	//getPrintGLEnum(GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS, "meh");
	//getPrintGLEnum(GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS, "meh");
	getPrintGLEnum(GL_MAX_TEXTURE_IMAGE_UNITS, "meh");
	getPrintGLEnum(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, "meh");
	getPrintGLEnum(GL_MAX_ARRAY_TEXTURE_LAYERS, "meh");
	getPrintGLEnum(GL_MAX_SAMPLES, "meh");
	getPrintGLEnum(GL_MAX_VERTEX_ATTRIBS, "meh");
	getPrintGLEnum(GL_MIN_PROGRAM_TEXEL_OFFSET, "meh");
	getPrintGLEnum(GL_MAX_PROGRAM_TEXEL_OFFSET, "meh");
	getPrintGLEnum(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, "meh");
	getPrintGLEnum(GL_MAX_UNIFORM_BLOCK_SIZE, "meh");
	glexit("");
	
	query_queue_init(&gs->queries.draw);
	
	// set up matrix stacks
	MatrixStack* view, *proj;
	
	view = &gs->view;
	proj = &gs->proj;
	
	msAlloc(2, view);
	msAlloc(2, proj);

	msIdent(view);
	msIdent(proj);
	

	
	
	// perspective matrix, pretty standard
// 	msPerspective(60, 1.0, 01000.0f, 100000.0f, proj);
// 		msOrtho(0, 1, 0, 1, .01, 100000, proj);

	gs->zoom = -760.0;
	gs->direction = 0.0f;
	gs->lookCenter.x = 128;
	gs->lookCenter.y = 128;
	
	
	//initStaticMeshes();
	
	// initialize all those magic globals
// 	initMap(&gs->map);
// 	scene_init(&gs->scene);
	
// 	initUI(gs);
	
		// maye intel needs this
	printf("gbuf init id: %d\n", gs->gbuf.fb);
	glexit("");
	//glBindFramebuffer(GL_FRAMEBUFFER, gs->gbuf.fb);
	glexit("fbo preinit");
	
	// text rendering stuff
	
	gui_Init();
	
	gt = guiTextNew("gui!", &(Vector){1.0,1.0,0.0}, 6.0f, "Arial");
	
	guiTextNew("gui2", &(Vector){10.0,1.0,0.0}, 6.0f, "Arial");
	
	
	
	axes_Init();
	
	OBJContents cube;
	loadOBJFile("assets/models/gazebo.obj", 0, &cube);
	//Mesh* cubem = OBJtoMesh(&cube);
	testmesh = staticMesh_FromOBJ(&cube);
	staticMesh_RegenMeta(testmesh);
	testrenderable = renderable_FromOBJ(&cube);
	
	testrenderable->scale = 150;
	
	int axisindex = axes_Add(10, NULL);
	
}




void preFrame(GameState* gs) {
	
	// update timers
	char frameCounterBuf[128];
	unsigned int fpsColors[] = {0xeeeeeeff, 6, 0xeeee22ff, INT_MAX};
	
	static int frameCounter = 0;
	static double last_frame = 0;
	static double lastPoint = 0;
	
	double now;
	
	gs->frameTime = now = getCurrentTime();
	
	if (last_frame == 0)
		last_frame = now;
	
	gs->frameSpan = (double)(now - last_frame);
	last_frame = now;
	
	frameCounter = (frameCounter + 1) % 60;
	
	// update per-frame timer values
	
	PerFrameUniforms* pfu = uniformBuffer_begin(&gs->perFrameUB);
	
	double seconds = (float)(long)gs->frameTime;
	
	pfu->wholeSeconds = seconds;
	pfu->fracSeconds = gs->frameTime - seconds;
	
	uniformBuffer_bindRange(&gs->perFrameUB);
	
	static double sdtime;
	
	if(lastPoint == 0.0f) lastPoint = gs->frameTime;
	if(1 /*frameCounter == 0*/) {
		float fps = 60.0f / (gs->frameTime - lastPoint);
		
		uint64_t qdtime;
		
		if(!query_queue_try_result(&gs->queries.draw, &qdtime)) {
			sdtime = ((double)qdtime) / 1000000.0;
		}
		
		
		glexit("");
		snprintf(frameCounterBuf, 128, "dtime:  %.2fms", sdtime);
		
		guiTextSetValue(gt, frameCounterBuf);
		lastPoint = now;
	}
	
	
	// check the pbos
	GLenum val;
	if(gs->selectionFence) {
		val = glClientWaitSync(gs->selectionFence, 0, 0);
		
		if(val == GL_CONDITION_SATISFIED || val == GL_ALREADY_SIGNALED) {
			printf("signaled %d\n", gs->frameCount - gs->selectionFrame);
			glDeleteSync(gs->selectionFence);
			gs->selectionFence = 0;
			
			glBindBuffer(GL_PIXEL_PACK_BUFFER, gs->selectionPBOs[gs->activePBO]);
			void* p = glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, 600*600*4, GL_MAP_READ_BIT);
			glexit("buffer map");
			if(p == NULL) {
				exit(1);
			}
			
			int sz = gs->screen.wh.x * gs->screen.wh.y * 4; 
			memcpy(gs->selectionData, p, sz);
			
			glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
			
			
			gs->readPBO = gs->activePBO;
			gs->activePBO = (gs->activePBO + 1) % 2;
		}
	}
}




void postFrame(GameState* gs) {
	
	double now;
	
	now = getCurrentTime();
	
	gs->perfTimes.draw = now - gs->frameTime;
	
	uniformBuffer_finish(&gs->perFrameUB);
}


void handleInput(GameState* gs, InputState* is) {
	
	double te = gs->frameSpan;

	float moveSpeed = gs->settings.keyScroll * te; // should load from config
	float rotateSpeed = gs->settings.keyRotate * te; // 20.8 degrees
	float keyZoom = gs->settings.keyZoom * te;
	float mouseZoom = gs->settings.mouseZoom * te;

	if(is->keyState[54] & IS_KEYDOWN) {
		exit(0);
	}
	
	if(is->clickButton == 1) {
		/*
		flattenArea(gs->map.tb,
			gs->cursorPos.x - 5,
			gs->cursorPos.y - 5,
			gs->cursorPos.x + 5,
			gs->cursorPos.y + 5
		);
		
		setZone(&gs->map,
			gs->cursorPos.x - 5,
			gs->cursorPos.y - 5,
			gs->cursorPos.x + 5,
			gs->cursorPos.y + 5,
			gs->activeTool + 1
		);
		
		
		checkMapDirty(&gs->map);
		*/
	}
	
	if(is->buttonDown == 1) {
// 		gs->mouseDownPos.x = gs->cursorPos.x;
// 		gs->mouseDownPos.y = gs->cursorPos.y;
// 		printf("start dragging at (%d,%d)\n", (int)gs->cursorPos.x, (int)gs->cursorPos.y);
	}
	if(is->buttonUp == 1) {
		//vCopy(&gs->cursorPos, &gs->mouseDownPos);
// 		printf("stopped dragging at (%d,%d)\n", (int)gs->cursorPos.x, (int)gs->cursorPos.y);
		
		
	}
	
	if(is->clickButton == 2) {
		gs->activeTool = (gs->activeTool + 1) % 3;
	}
	
	// look direction
	if(is->keyState[38] & IS_KEYDOWN) {
		gs->direction += rotateSpeed;
		gs->hasMoved = 1;
	}
	if(is->keyState[39] & IS_KEYDOWN) {
		gs->direction -= rotateSpeed;
		gs->hasMoved = 1;
	}

	// keep rotation in [0,F_2PI)
	gs->direction = fmodf(F_2PI + gs->direction, F_2PI);
	
	// zoom
	if(is->keyState[52] & IS_KEYDOWN) { 
		gs->zoom += keyZoom;
 		gs->zoom = fmin(gs->zoom, -10.0);
		gs->hasMoved = 1;
	}
	if(is->keyState[53] & IS_KEYDOWN) {
		gs->zoom -= keyZoom;
		gs->hasMoved = 1;
	}
	if(is->clickButton == 4) {
		gs->zoom += mouseZoom;
 		gs->zoom = fmin(gs->zoom, -10.0);
		gs->hasMoved = 1;
	}
	if(is->clickButton == 5) {
		gs->zoom -= mouseZoom;
		gs->hasMoved = 1;
	}

	// movement
	Vector move = {
		.x = moveSpeed * sin(F_PI - gs->direction),
		.y = moveSpeed * cos(F_PI - gs->direction),
		.z = 0.0f
	};
	
	if(is->keyState[111] & IS_KEYDOWN) {
		vAdd(&gs->lookCenter, &move, &gs->lookCenter);
		gs->hasMoved = 1;
	}
	if(is->keyState[116] & IS_KEYDOWN) {
		vSub(&gs->lookCenter, &move, &gs->lookCenter);
		gs->hasMoved = 1;
	}
	
	// flip x and y to get ccw normal, using move.z as the temp
	move.z = move.x;
	move.x = -move.y;
	move.y = move.z;
	move.z = 0.0f;
	
	if(is->keyState[113] & IS_KEYDOWN) {
		vSub(&gs->lookCenter, &move, &gs->lookCenter);
		gs->hasMoved = 1;
	}
	if(is->keyState[114] & IS_KEYDOWN) {
		vAdd(&gs->lookCenter, &move, &gs->lookCenter);
		gs->hasMoved = 1;
	}
	
	// these don't stimulate a selection pass since they are debug tools
	if(is->keyState[110] & IS_KEYDOWN) {
		gs->nearClipPlane += 50 * te;
		printf("near: %f, far: %f\n", gs->nearClipPlane, gs->farClipPlane);
	}
	if(is->keyState[115] & IS_KEYDOWN) {
		gs->nearClipPlane -= 50 * te;
		printf("near: %f, far: %f\n", gs->nearClipPlane, gs->farClipPlane);
	}
	if(is->keyState[112] & IS_KEYDOWN) {
		gs->farClipPlane += 250 * te;
		printf("near: %f, far: %f\n", gs->nearClipPlane, gs->farClipPlane);
	}
	if(is->keyState[117] & IS_KEYDOWN) {
		gs->farClipPlane -= 250 * te;
		printf("near: %f, far: %f\n", gs->nearClipPlane, gs->farClipPlane);
	}
	
	static lastChange = 0;
	if(is->keyState[119] & IS_KEYDOWN) {
		if(gs->frameTime > lastChange + 1) {
			gs->debugMode = (gs->debugMode + 1) % 5;
			lastChange = gs->frameTime;
		}
	}
	
	
}


void setGameSettings(GameSettings* g, UserConfig* u) {
	
	const float rotateFactor = 0.7260f;
	const float scrollFactor = 300.0f;
	const float zoomFactor = 600.0f;
	
	g->keyRotate = rotateFactor * fclampNorm(u->keyRotateSensitivity);
	g->keyScroll = scrollFactor * fclampNorm(u->keyScrollSensitivity);
	g->keyZoom = zoomFactor * fclampNorm(u->keyZoomSensitivity);
	
	g->mouseRotate = rotateFactor * fclampNorm(u->mouseRotateSensitivity);
	g->mouseScroll = scrollFactor * fclampNorm(u->mouseScrollSensitivity);
	g->mouseZoom = 4 * zoomFactor * fclampNorm(u->mouseZoomSensitivity);
	
	printf("keyRotate %.3f\n", g->keyRotate);
	printf("keyScroll %.3f\n", g->keyScroll);
	printf("keyZoom %.3f\n", g->keyZoom);
	printf("mouseRotate %.3f\n", g->mouseRotate);
	printf("mouseScroll %.3f\n", g->mouseScroll);
	printf("mouseZoom %.3f\n", g->mouseZoom);
	
}


void setUpView(GameState* gs) {
	
	
}


// actually the selection pass
void depthPrepass(XStuff* xs, GameState* gs, InputState* is) {
	
	// draw UI
	//renderUIPicking(xs, gs);
	
	
	
	//updateView(xs, gs, is);
	

	// draw terrain
// 	drawTerrainBlockDepth(&gs->map, msGetTop(&gs->model), msGetTop(&gs->view), msGetTop(&gs->proj));
//	drawTerrainDepth(&gs->scene.map, &gs->perViewUB, &gs->screen.wh);
	
	//msPop(&gs->view);
	//msPop(&gs->proj);
	
}


void updateView(XStuff* xs, GameState* gs, InputState* is) {
		
	msPush(&gs->proj);
	msPerspective(60, gs->screen.aspect, gs->nearClipPlane, gs->farClipPlane, &gs->proj);
	
	
	msPush(&gs->view);
	
	// order matters! don't mess with this.
	msTrans3f(0, -1, gs->zoom, &gs->view);
	msRot3f(1, 0, 0, F_PI / 6, &gs->view);
	msRot3f(0,1,0, gs->direction, &gs->view);
	msTrans3f(-gs->lookCenter.x, 0, -gs->lookCenter.y, &gs->view);
	
	// y-up to z-up rotation
	msRot3f(1, 0, 0, F_PI_2, &gs->view);
	msScale3f(1, 1, -1, &gs->view);
	
	
	
	// calculate cursor position
	Vector cursorp;
	Vector eyeCoord;
	Vector worldCoord;
	Matrix p, invp, invv;
	
	// device space (-1:1)
	Vector devCoord;
	devCoord.x = 0.50;
	devCoord.y = 0.50;
	devCoord.z = -1.0;
	
	// eye space
	mInverse(msGetTop(&gs->proj), &invp);
	vMatrixMul(&devCoord, &invp, &eyeCoord);
	vNorm(&eyeCoord, &eyeCoord);
	
	// world space
	mInverse(msGetTop(&gs->view), &invv);
	vMatrixMul(&eyeCoord, &invv, &worldCoord);
	vNorm(&worldCoord, &worldCoord);
	
	// TODO: only update if somethign changes
	PerViewUniforms* pvu = uniformBuffer_begin(&gs->perViewUB);
	
	memcpy(&pvu->view , msGetTop(&gs->view), sizeof(Matrix));
	memcpy(&pvu->proj , msGetTop(&gs->proj), sizeof(Matrix));
	
	uniformBuffer_bindRange(&gs->perViewUB);
}


void cleanUpView(XStuff* xs, GameState* gs, InputState* is) {
	msPop(&gs->view);
	msPop(&gs->proj);
	
	uniformBuffer_finish(&gs->perFrameUB);
	uniformBuffer_finish(&gs->perViewUB);
}

void renderFrame(XStuff* xs, GameState* gs, InputState* is) {
	
	//mModel = IDENT_MATRIX;
	
	Vector2 c2;
	
	c2.x = 300; //cursorp.x;
	c2.y = 300; //cursorp.z;
	
	//updateView(xs, gs, is);

// 	drawStaticMesh(testmesh, msGetTop(&gs->view), msGetTop(&gs->proj));
	renderable_Draw(testrenderable, msGetTop(&gs->view), msGetTop(&gs->proj));


	guiRenderAll(gs);
}




void shadingPass(GameState* gs) {
	
	Matrix world;
	
	world = IDENT_MATRIX;
	
	glUseProgram(shadingProg->id);
	glexit("shading prog");

	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, gs->diffuseTexBuffer);
	
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, gs->normalTexBuffer);

	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, gs->depthTexBuffer);

	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, gs->selectionTexBuffer);

	
	glUniform1i(glGetUniformLocation(shadingProg->id, "debugMode"), gs->debugMode);
	glUniform2f(glGetUniformLocation(shadingProg->id, "clipPlanes"), gs->nearClipPlane, gs->farClipPlane);
	
	glexit("shading samplers");
	
// 	glUniformMatrix4fv(glGetUniformLocation(shadingProg->id, "world"), 1, GL_FALSE, world.m);
// 	glexit("shading world");
// msGetTop(&gs->view), 
	glUniformMatrix4fv(glGetUniformLocation(shadingProg->id, "mViewProj"), 1, GL_FALSE, msGetTop(&gs->proj)->m);
	glUniformMatrix4fv(glGetUniformLocation(shadingProg->id, "mWorldView"), 1, GL_FALSE, msGetTop(&gs->view)->m);
	glexit("shading world");

	//glUniform3fv(glGetUniformLocation(shadingProg->id, "sunNormal"), 1, (float*)&gs->sunNormal);
	
	if(gs->screen.resized) {
		glUniform2fv(glGetUniformLocation(shadingProg->id, "resolution"), 1, (float*)&gs->screen.wh);
	}
	
	drawFSQuad();
	glexit("post quad draw");
}

	
Vector2i viewWH = {
	.x = 0,
	.y = 0
};
void checkResize(XStuff* xs, GameState* gs) {
	if(viewWH.x != xs->winAttr.width || viewWH.y != xs->winAttr.height) {
		
		// TODO: destroy all the textures too
		
		printf("screen 0 resized\n");
		
		viewWH.x = xs->winAttr.width;
		viewWH.y = xs->winAttr.height;
		
		gs->screen.wh.x = (float)xs->winAttr.width;
		gs->screen.wh.y = (float)xs->winAttr.height;
		
		gs->screen.aspect = gs->screen.wh.x / gs->screen.wh.y;
		
		gs->screen.resized = 1;
		
		setupFBOs(gs, 1);
		
		printf("diffuse2: %d\n",gs->diffuseTexBuffer);
		printf("resize finished\n");
	}
}

#define PF_START(x) gs->perfTimes.x = getCurrentTime()
#define PF_STOP(x) gs->perfTimes.x = timeSince(gs->perfTimes.x)

void gameLoop(XStuff* xs, GameState* gs, InputState* is) {
	gs->frameCount++;
	
	checkResize(xs,gs);
	
		PF_START(preframe);
	preFrame(gs);
		PF_STOP(preframe);
	
	handleInput(gs, is);
	
	//setUpView(gs);
	updateView(xs, gs, is);
	
	// disable selection pass because intel gpus suck
	if(0 && gs->hasMoved && gs->lastSelectionFrame < gs->frameCount - 8) {
		printf("doing selection pass %d\n", gs->frameCount);
		gs->hasMoved = 0;
		gs->lastSelectionFrame = gs->frameCount; 
		
		// really just the selection pass
			PF_START(selection);
		glDepthFunc(GL_LESS);
		//glDisable(GL_DEPTH_TEST);
		
		glBindFramebuffer(GL_FRAMEBUFFER, gs->selectionbuf.fb);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		depthPrepass(xs, gs, is);
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, gs->selectionbuf.fb);
		
		//glEnable(GL_DEPTH_TEST);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		
		printf("is buffer %d\n", gs->selectionPBOs[gs->activePBO]);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, gs->selectionPBOs[gs->activePBO]);
		
		glexit("selection buff");
		//printf("cursor pixels: %f, %f\n", is->cursorPosPixels.x, is->cursorPosPixels.y);
		
		glReadPixels(
			0, //is->cursorPosPixels.x,
			0, //is->cursorPosPixels.y,
			gs->screen.wh.x,
			gs->screen.wh.y,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			0);
		
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

		glexit("read selection");
		glexit("");
		
		if(gs->selectionFence) glDeleteSync(gs->selectionFence);
		gs->selectionFence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		
		gs->selectionFrame = gs->frameCount;

		
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
			PF_STOP(selection);
			
	}
	
	
// 	checkCursor(gs, is);
	// update world state
// 	glBeginQuery(GL_TIME_ELAPSED, gs->queries.dtime[gs->queries.dtimenum]);
	query_queue_start(&gs->queries.draw);

	
	glBindFramebuffer(GL_FRAMEBUFFER, gs->gbuf.fb);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
		
		PF_START(draw);
		//static int fnum = 0;
		//printf("frame %d\n", fnum++);
		
		glexit("");
	
	// clear color buffer for actual rendering
	//glClear(GL_COLOR_BUFFER_BIT);
	glerr("pre shader create 1d");
	glDepthFunc(GL_LEQUAL);
	glerr("pre shader create e");
	
	renderFrame(xs, gs, is);
	
		
		PF_STOP(draw);
		PF_START(decal);

	// decals
	glBindFramebuffer(GL_FRAMEBUFFER, gs->decalbuf.fb);
	
	glDepthMask(GL_FALSE); // disable depth writes for decals
	
// 	renderDecals(xs, gs, is);
	
// 	renderParticles(xs, gs, is); // particles are alpha blended and need to be on top of decals
	
	glDepthMask(GL_TRUE);
		
		PF_STOP(decal);
	
	
	cleanUpView(xs, gs, is);
	
	// draw to the screen
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	
	shadingPass(gs);
	
// 	renderUI(xs, gs);
	
	gs->screen.resized = 0;

	postFrame(gs);
	//glEndQuery(GL_TIME_ELAPSED);
	
	
	query_queue_stop(&gs->queries.draw);
	
	
	glXSwapBuffers(xs->display, xs->clientWin);

	
	
}







