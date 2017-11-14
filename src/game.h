
#ifndef __EACSMB_GAME_H__
#define __EACSMB_GAME_H__

#include "config.h" // UserConfig
#include "uniformBuffer.h"
#include "fbo.h"
#include "timing.h"
#include "staticMesh.h"
//#include "scene.h"


typedef struct UserConfig {
	
	float keyRotateSensitivity;
	float keyScrollSensitivity;
	float keyZoomSensitivity;
	
	float mouseRotateSensitivity;
	float mouseScrollSensitivity;
	float mouseZoomSensitivity;
	
} UserConfig;



typedef struct GameScreen {
	
	float aspect;
	Vector2 wh;
	
	int resized;
	
} GameScreen;


typedef struct GameSettings {
	
	float keyRotate;
	float keyScroll;
	float keyZoom;
	
	float mouseRotate;
	float mouseScroll;
	float mouseZoom;
	
} GameSettings;




typedef struct PerViewUniforms {
	Matrix view;
	Matrix proj;
} PerViewUniforms;


typedef struct PerFrameUniforms {
	float wholeSeconds;
	float fracSeconds;
} PerFrameUniforms;


typedef struct GameState {
	
	GameScreen screen;
	
	GameSettings settings;
	UserConfig uSettings;
	
	GLuint diffuseTexBuffer, normalTexBuffer, depthTexBuffer, selectionTexBuffer, lightingTexBuffer;
	GLuint framebuffer;
	GLuint depthRenderbuffer;
	
	GLuint* fboTextures; 
	Framebuffer gbuf;
	Framebuffer selectionbuf;
	Framebuffer decalbuf;
	Framebuffer lightingbuf;
	
	uint32_t* selectionData;
	uint64_t selectionFrame;
	GLsync selectionFence;
	GLuint selectionPBOs[2];
	char readPBO, activePBO;
	
//	Scene scene;
	
	UniformBuffer perViewUB;
	UniformBuffer perFrameUB;
	
	MatrixStack view;
	MatrixStack proj;
	
	double nearClipPlane;
	double farClipPlane;
	
	Vector eyePos;
	Vector eyeDir;
	Vector eyeUp;
	Vector eyeRight;
	
	StaticMesh* activeMesh;

	Vector2 mouseDownPos;
	
	int debugMode;
	
	float zoom;
	float direction;
	Vector lookCenter;

	

	
	double frameTime; // ever incrementing time of the this frame
	double frameSpan; // the length of this frame, since last frame
	uint64_t frameCount; // ever incrementing count of the number of frames processed
	
	// performance counters
	struct {
		double preframe;
		double selection;
		double draw;
		double decal;
		double light;
		double shade;
		
	} perfTimes;
	
	struct {
		QueryQueue draw; 
		
	} queries;

	
	// info for the selection pass
	char hasMoved; // if the view has moved since the last selection pass
	uint64_t lastSelectionFrame; // frame number of the last time a selection pass was rendered
	
	
	// temp stuff with no better place atm
	int activeTool;
	
	
	Renderable* renderable;
	
	
} GameState;








// use a normal map to not have the overlap problem




void renderFrame(XStuff* xs, GameState* gs, InputState* is);
void gameLoop(XStuff* xs, GameState* gs, InputState* is);

void setGameSettings(GameSettings* g, UserConfig* u);

void loadSettings(char* path, GameState* gs);

#endif // __EACSMB_GAME_H__
