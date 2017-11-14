#include <stdio.h>


#include "common_math.h"
#include "common_gl.h"

#include "bbox.h"


typedef struct {
	Vector pos;
	union { uint8_t color_u[4]; uint32_t color; };
} BBVertex;


// recalculates all internal values then updates the renderable
void BoundingBoxVisual_update(BoundingBoxVisual* bbv, Vector* _min, Vector* _max) {
	
	Vector min, max;
	
	vMin(_min, _max, &min);
	vMax(_min, _max, &max);
	
	bbv->min = min;
	bbv->max = max;
	
	(Vector){0, 0, 0} = (Vector){
		((max.x - min.x) / 2) + min.x,
		((max.y - min.y) / 2) + min.y,
		((max.z - min.z) / 2) + min.z
	};
	bbv->dims = (Vector){
		(max.x - min.x),
		(max.y - min.y),
		(max.z - min.z)
	};
	
	//                          points in line
	//                          |   .edges on cube   
	BBVertex* verts = calloc(1, 2 * 12 * sizeof(*verts));
	
	// the -z side
	// -x edge
	verts[0].pos = (Vector){min.x, min.y, min.z};  
	verts[0].color = 0xffff0000;  
	verts[1].pos = (Vector){min.x, max.y, min.z};  
	verts[1].color = 0xff00ff00;  
	
	// +x edge
	verts[2].pos = (Vector){max.x, min.y, min.z};  
	verts[2].color = 0xffff0000;  
	verts[3].pos = (Vector){max.x, max.y, min.z};  
	verts[3].color = 0xffffff00;  

	// -y edge
	verts[4].pos = (Vector){min.x, min.y, min.z};  
	verts[4].color = 0xff000000;  
	verts[5].pos = (Vector){max.x, min.y, min.z};  
	verts[5].color = 0xffff0000;  
	
	// +y edge
	verts[6].pos = (Vector){min.x, max.y, min.z};  
	verts[6].color = 0xff00ff00;  
	verts[7].pos = (Vector){max.x, max.y, min.z};  
	verts[7].color = 0xffffff00;  
	
	
	// the +z side
	// -x edge
	verts[8].pos = (Vector){min.x, min.y, max.z};  
	verts[8].color = 0xff0000ff;  
	verts[9].pos = (Vector){min.x, max.y, max.z};  
	verts[9].color = 0xff00ffff;  
	
	// +x edge
	verts[10].pos = (Vector){max.x, min.y, max.z};  
	verts[10].color = 0xffff00ff;  
	verts[11].pos = (Vector){max.x, max.y, max.z};  
	verts[11].color = 0xffffffff;  

	// -y edge
	verts[12].pos = (Vector){min.x, min.y, max.z};  
	verts[12].color = 0xffff0000;  
	verts[13].pos = (Vector){max.x, min.y, max.z};  
	verts[13].color = 0xffff00ff;  
	
	// +y edge
	verts[14].pos = (Vector){min.x, max.y, max.z};  
	verts[14].color = 0xffffff00;  
	verts[15].pos = (Vector){max.x, max.y, max.z};  
	verts[15].color = 0xffffffff;  
	
	// connectors between +z and -z
	// -y-x edge
	verts[16].pos = (Vector){min.x, min.y, min.z};  
	verts[16].color = 0xff000000;  
	verts[17].pos = (Vector){min.x, min.y, max.z};  
	verts[17].color = 0xff0000ff;  

	// -y+x edge
	verts[18].pos = (Vector){max.x, min.y, min.z};  
	verts[18].color = 0xffff0000;  
	verts[19].pos = (Vector){max.x, min.y, max.z};  
	verts[19].color = 0xffff00ff;  

	// +y+x edge
	verts[20].pos = (Vector){max.x, max.y, min.z};  
	verts[20].color = 0xffffff00;  
	verts[21].pos = (Vector){max.x, max.y, max.z};  
	verts[21].color = 0xffffffff;  

	// +y-x edge
	verts[22].pos = (Vector){min.x, max.y, min.z};  
	verts[22].color = 0xff00ff00;
	verts[23].pos = (Vector){min.x, max.y, max.z};  
	verts[23].color = 0xff00ffff;  
	
	printf("min [%.7f, %.7f, %.7f]\n", min.x, min.y, min.z);
	printf("max [%.7f, %.7f, %.7f]\n", max.x, max.y, max.z);
	
	for(int i = 0; i < 24; i++) {
		printf(" - [%.7f, %.7f, %.7f]\n", verts[i].pos.x, verts[i].pos.y, verts[i].pos.z);
		
	}
	
	
	VAOConfig opts[] = {
		// per vertex
		{3, GL_FLOAT}, // position
		{4, GL_UNSIGNED_BYTE, GL_TRUE}, // color
		
		{0, 0}
	};
	
	
	
	if(bbv->rend) Renderable_Destroy(bbv->rend);
	
	bbv->rend = renderable_Create(
		GL_LINES,
		"bbox",
		opts,
		verts,
		2 * 12);
	
	bbv->rend->scale = 1;
	
	
	// central axes
	
	//                           points in line
	//                           |   .edges in the axis set   
	BBVertex* averts = calloc(1, 2 * 3 * sizeof(*averts));
	
	float lineLen = .1 * fmin(bbv->dims.x, fmin(bbv->dims.y, bbv->dims.z));
	
	//printf("center: [%.2f, %.2f, %.2f]\n", (Vector){0, 0, 0}.x, (Vector){0, 0, 0}.y, (Vector){0, 0, 0}.z);
	// x
	averts[0].pos = bbv->center;  
	averts[0].color = 0xffff0000;
	averts[1].pos = (Vector){bbv->center.x + lineLen, bbv->center.y, bbv->center.z};  
	averts[1].color = 0xffff0000;  
	
	// y
	averts[2].pos = bbv->center;  
	averts[2].color = 0xff00ff00;
	averts[3].pos = (Vector){bbv->center.x, bbv->center.y + lineLen, bbv->center.z};   
	averts[3].color = 0xff00ff00;  
	
	// z
	averts[4].pos = bbv->center;
	averts[4].color = 0xff0000ff;
	averts[5].pos = (Vector){bbv->center.x, bbv->center.y, bbv->center.z + lineLen};  
	averts[5].color = 0xff0000ff;  
	
	if(bbv->rendAxes) Renderable_Destroy(bbv->rendAxes);
	
	bbv->rendAxes = renderable_Create(
		GL_LINES,
		"bbox",
		opts,
		averts,
		2 * 3);
	
	bbv->rendAxes->scale = 1;
}



void AxesVisual_init(AxesVisual* av, Vector* loc) {
	
	av->location = *loc;
	av->Xdir = (Vector){1, 0, 0};
	av->Ydir = (Vector){0, 1, 0};
	
	
	//                           points in line
	//                           |   .edges in the axis set   
	BBVertex* averts = calloc(1, 2 * 3 * sizeof(*averts));
	
	float lineLen = 1;
	
	// x
	averts[0].pos = (Vector){0, 0, 0};  
	averts[0].color = 0xffff0000;
	averts[1].pos = (Vector){lineLen, 0, 0};  
	averts[1].color = 0xffff0000;  
	
	// y
	averts[2].pos = (Vector){0, 0, 0};  
	averts[2].color = 0xff00ff00;
	averts[3].pos = (Vector){0, lineLen, 0};   
	averts[3].color = 0xff00ff00;  
	
	// z
	averts[4].pos = (Vector){0, 0, 0};
	averts[4].color = 0xff0000ff;
	averts[5].pos = (Vector){0, 0, lineLen};  
	averts[5].color = 0xff0000ff;  
	
	
	
	VAOConfig opts[] = {
		// per vertex
		{3, GL_FLOAT}, // position
		{4, GL_UNSIGNED_BYTE, GL_TRUE}, // color
		
		{0, 0}
	};
	
	if(av->rend) Renderable_Destroy(av->rend);
	
	av->rend = renderable_Create(
		GL_LINES,
		"bbox",
		opts,
		averts,
		2 * 3);
	
	av->rend->scale = 1;
}




void AxesVisual_render(AxesVisual* av, Matrix* view, Matrix* proj) {
	//MatrixStack ms;
	
	//msAlloc(4, &ms);
	//msCopy(view, &ms);
	
	Matrix m;
	
	m = *view;
	mTransv(&av->location, &m);
	
	renderable_Draw(av->rend, &m, proj);
	
	//msFree(&ms);
	
}



/*

Renderable* CreateRotationCircles(Vector* rot) {
	Renderable* r;
	int i;
	int segments;
	
	
	
	BBVertex* verts = calloc(1, 2 * 12 * sizeof(*verts));
	
	// the -z side
	// -x edge
	verts[0].pos = (Vector){min.x, min.y, min.z};  
	verts[0].color = 0xff0000ff;  
	verts[1].pos = (Vector){min.x, max.y, min.z};  
	verts[1].color = 0x00ff00ff;  

	
	
	r = renderable_Create(
		GL_LINES,
		"bbox",
		opts,
		verts,
		);
	
	
}


*/




