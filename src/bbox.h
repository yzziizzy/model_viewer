#ifndef __model_viewer__bbox_h__
#define __model_viewer__bbox_h__



#include "ds.h"
#include "renderable.h"



typedef struct BoundingBoxVisual {
	
	Vector min, max;
	
	Vector center;
	Vector dims;
	
	Renderable* rend;
	Renderable* rendAxes;
	// todo: dimensions text
	
	char showAxes;
	char showDimensions;
	char showLocation;
	
} BoundingBoxVisual;





typedef struct AxesVisual {
	Vector location;
	Vector Xdir;
	Vector Ydir;
	
	Renderable* rend;
	
} AxesVisual;






#endif // __model_viewer__bbox_h__
