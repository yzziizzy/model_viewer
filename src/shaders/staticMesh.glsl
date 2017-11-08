
#shader VERTEX


#version 330 core

layout (location = 0) in vec3 pos_in;
layout (location = 1) in vec3 norm_in;
layout (location = 2) in vec2 tex_in;


uniform mat4 mModel;
uniform mat4 mView;
uniform mat4 mProj;

// out vec4 vs_pos;
out vec4 vs_norm;
out vec2 vs_tex;

void main() {
	gl_Position = (mProj * mView * mModel) * vec4(pos_in, 1);
	vs_norm = vec4(norm_in, 1); // normalize(vec4(1,1,1,0));
	vs_tex = tex_in;
}




#shader FRAGMENT

#version 330

// in vec4 vs_pos;
in vec4 vs_norm;
in vec2 vs_tex;

// fragment shader
uniform vec4 color;
uniform sampler2D tex; 

layout(location = 0) out vec4 out_Color;
layout(location = 1) out vec4 out_Normal;
//layout(location = 2) out vec4 out_Lighting;


void main(void) {
	vec2 tc = vs_tex.xy;
	
	out_Color = vec4(texture(tex, vec2(tc.x, 1-tc.y)).xyz, 1); //vs_norm;
	//out_Color = vec4(tc.xy, 1, 1); //vs_norm;
	out_Normal = normalize(vec4(0.5, 0.5, 0.5, 1)); //vs_norm;
//	out_Lighting = vec4(0.5, 0.5, 1, 1);
}

