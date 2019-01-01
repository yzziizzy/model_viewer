
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "collada.h"


#include "fxml/fxml.h"





float* parseFloatArray(FXMLTag* tag, int* cnt) {
	
	char* e, *s, *raw;
	size_t count, i;
	float* data, *d;
	
	count = fxmlGetAttrInt(tag, "count");
	if(!count) {
		fprintf(stderr, "Collada: empty float array.\n");
		*cnt = 0;
		return NULL;
	}
	
	data = d = calloc(1, count * sizeof(*data));
	//CHECK_OOM(data);
	
	raw = s = fxmlGetTextContents(tag, NULL);
	
	i = 0;
	while(*s) {
		*d = strtod(s, &e);
		if(s == e) {
			if(count != i) {
				fprintf(stderr, "Collada: float_array count attribute disagrees with actual data count.\n");
			}
			break;
		}
		
		s = e;
		d++;
		i++;
	}
	
	free(raw);
	
	*cnt = i;
	return data;
}


void parseTriangles(FXMLTag* tag) {
	FXMLTag* x_p, *x_in;
	char* e, *s, *raw;
	size_t count, i;
	float* data, *d;
	
	count = fxmlGetAttrInt(tag, "count");
	if(!count) {
		fprintf(stderr, "Collada: empty float array.\n");
		return NULL;
	}
	
	
	// find the storage configuration
	x_in = fxmlTagFindFirstChild(tag, "input");
	for(int i = 0; x_in; i++) {
		
// 		char* p = fxmlGetAttr(x_p, "param");
// 		src->access[i] = p[0];
// 		free(p);
		
		x_in = fxmlTagFindNextSibling(x_p, "input", 1);
	}
	
	
	// grab the polygon data
	data = d = calloc(1, count * sizeof(*data));
	
	x_p = fxmlTagFindFirstChild(tag, "p");
	raw = s = fxmlGetTextContents(x_p, NULL);
	
	i = 0;
	while(*s) {
		*d = strtod(s, &e);
		if(s == e) {
			if(count != i) {
				fprintf(stderr, "Collada: float_array count attribute disagrees with actual data count.\n");
			}
			break;
		}
		
		s = e;
		d++;
		i++;
	}
	
	free(raw);
	
	
}

/*
	<source id="color_source" name="Colors">
		<float_array id="values" count="3">
			0.8 0.8 0.8
		</float_array>
		<technique_common>
			<accessor source="#values" count="1" stride="3">
				<param name="R" type="float"/>
				<param name="G" type="float"/>
				<param name="B" type="float"/>
			</accessor>
		</technique_common>
	</source>
*/
static void parseSource(FXMLTag* srctag) {
	FXMLTag* datatag, *tc, *acc;
	
	char* datanames[] = {
		"bool_array",
		"float_array",
		"IDREF_array",
		"int_array",
		"Name_array",
		"SIDREF_array",
		"token_array",
		NULL
	};
	
	//fetches the next sibling with any of the specified names, or null if it doesn't exist.
	datatag = fxmlTagFindFirstChildArray(srctag, datanames);
	if(!datatag) {
		fprintf(stderr, "Collada: no valid data tag found in source tag.\n");
		return;
	}
	
	// count, name? id?
	
	
	
	// technique_common
	tc = fxmlTagFindFirstChild(srctag, "technique_common");
	if(!tc) {
		fprintf(stderr, "Collada: no technique_common tag found in source tag.\n");
		return;
	}
	
	// accessor
	acc = fxmlTagFindFirstChild(srctag, "accessor");
	if(!acc) {
		fprintf(stderr, "Collada: no accessor tag found in technique_common tag.\n");
		return;
	}
	
	/*
	while() {
		// <param> tags
	}
	*/
	
	
	
	
	
}


static ColladaMesh* parseMesh(FXMLTag* meshtag) {
	
	ColladaMesh* cm;
	
	cm = calloc(1, sizeof(*cm));
	
		
		// multiple <source
		// one <vertices
	
	
}



static void parseGeomSourceTag(ColladaMesh* cm, FXMLTag* x_src) {
	ColladaGeomSource* src;
	FXMLTag* x_fa, *x_tc, *x_acc, *x_p;
	
	src = calloc(1, sizeof(*src));
	
	src->id = fxmlGetAttr(x_src, "id");
	
	// float data
	x_fa = fxmlTagFindFirstChild(x_src, "float_array");
	if(!x_fa) {
		printf("float aray not found in mesh source\n");
		return;
	}
	
	parseFloatArray(x_fa, &src->count);
	
	
	
	x_tc = fxmlTagFindFirstChild(x_src, "technique_common");
	if(!x_tc) {
		fprintf(stderr, "Collada: no technique_common tag found in source tag.\n");
		return;
	}
	x_acc = fxmlTagFindFirstChild(x_tc, "accessor");
	if(!x_acc) {
		fprintf(stderr, "Collada: no accessor tag found in technique_common tag.\n");
		return;
	}
	
	src->stride = fxmlGetAttrInt(x_acc, "stride");
	
	// find the storage order
	x_p = fxmlTagGetFirstChild(x_acc);
	for(int i = 0; x_p && i < 4; i++) {
		
		char* p = fxmlGetAttr(x_p, "param");
		src->access[i] = p[0];
		free(p);
		
		x_p = fxmlTagNextSibling(x_p, 1);
	}
	
	
	HT_set(&cm->sources, src->id, src);
}

static void parseGeomTag(ColladaFile* cf, FXMLTag* geom) {
	printf("parsing geometry\n");
	
	ColladaGeometry* g;
	g = calloc(1, sizeof(*g));
	
	
	g->name = fxmlGetAttr(geom, "name");
	g->id = fxmlGetAttr(geom, "id");
	
	FXMLTag* x_mesh = fxmlTagFindFirstChild(geom, "mesh");
	if(!x_mesh) {
		printf("No mesh found in geometry\n");
		return;
	}
	
	if(x_mesh) {
		ColladaMesh* cm;
		cm = calloc(1, sizeof(*cm));
		HT_init(&cm->sources, 2);
		
		char has_vertices = 0;
		
		cm->id = fxmlGetAttr(x_mesh, "id");
		
		
		FXMLTag* tag = fxmlTagGetFirstChild(x_mesh);
		
		while(tag) {
			if(0 == strncmp("source", tag->name, tag->name_len)) {
				parseGeomSourceTag(cm, tag);
			}
			else if(0 == strncmp("vertices", tag->name, tag->name_len)) {
				printf("NIH geom/mesh/vertices\n");
				
				has_vertices = 1;
				
				
				
			}
			else if(0 == strncmp("triangles", tag->name, tag->name_len)) {
				printf("NIH geom/mesh/triangles\n");
			}
			
			tag = fxmlTagNextSibling(tag, 1);
		}
		
		if(!has_vertices) {
			printf("mesh lacks vertices\n");
		}
	
		g->mesh = cm;
	}
	
	
	HT_set(&cf->geometries, g->id, g);
}







void colladaLoadFile(char* path) {
	FXMLFile* xml;
	FXMLTag* tag, *tag2, *tmp_tag;
	char* str;
	ColladaFile* cf;
	
	cf = calloc(1, sizeof(*cf));
	HT_init(&cf->geometries, 2);
	
	xml = fxmlLoadFile(path);
	
	tag = fxmlTagGetFirstChild(xml->root);
	
	
	while(tag) {
		if(0 == strncmp("asset", tag->name, tag->name_len)) {
			printf("found asset tag\n");
			tmp_tag = fxmlTagFindFirstChild(tag, "up_axis");
			if(tmp_tag) {
				str = fxmlGetTextContents(tmp_tag, NULL);
				if(0 == strcasecmp(str, "X_UP")) {
					cf->upAxis = 'x';
				}
				else if(0 == strcasecmp(str, "Y_UP")) {
					cf->upAxis = 'y';
				}
				else if(0 == strcasecmp(str, "Z_UP")) {
					cf->upAxis = 'z';
				}
				else {
					printf("COLLADA: unknown up axis: '%s'\n", str);
				}
				
				free(str);
			}
		}
		else if(0 == strncmp("library_geometries", tag->name, tag->name_len)) {
			printf("found geom library tag\n");
			
			tmp_tag = fxmlTagFindFirstChild(tag, "geometry");
			while(tmp_tag) {
				parseGeomTag(cf, tmp_tag);
				tmp_tag = fxmlTagFindNextSibling(tmp_tag, "geometry", 1);
				printf("v> %p\n", tmp_tag);
			}
			printf("finished geom\n");
		}
		else if(0 == strncmp("library_visual_scenes", tag->name, tag->name_len)) {
			printf("found vis. scene tag\n");
			// TODO: finish
		}
		printf("tag: %.*s\n", tag->name_len, tag->name);
		tag = fxmlTagNextSibling(tag, 1);
		printf("w> %p\n", tag);
	}
	
	
	/*
	// pull out each mesh
	while(1) {
		FXMLTag* geom;
		ColladaGeometry* cg;
		
		
		geom = fxmlFindFirstChild(libgeom, "geometry");
		cg = fxmlGetAttr(geom, "name");
		
		mesh = fxmlFindFirstChild(geom, "mesh");
		if(!mesh) {
			// this geomotry tag is an unupported type
			
			// free the shit
			
			continue;
		}
		
		
		
		colladaParseMesh(mesh);

		
	}
	*/
}









