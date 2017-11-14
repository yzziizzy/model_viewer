
#shader VERTEX


#version 330 core

layout (location = 0) in vec3 pos_in;
layout (location = 1) in vec4 color_in;


uniform mat4 mModel;
uniform mat4 mView;
uniform mat4 mProj;

// out vec4 vs_pos;
out vec4 vs_color;

void main() {
	gl_Position = (mProj * mView * mModel) * vec4(pos_in, 1);
	vs_color = color_in;
}




#shader FRAGMENT

#version 330

in vec4 vs_color;


layout(location = 0) out vec4 out_Color;
layout(location = 1) out vec4 out_Normal;
//layout(location = 2) out vec4 out_Lighting;


void main(void) {
	
// 	out_Color = vec4(vs_color.xyz, 1); //vs_norm;
	out_Color = vec4(vs_color.xyz / 256, 1); //vs_norm;
	//out_Color = vec4(tc.x, 0, 0, 1); //vs_norm;
	out_Normal = normalize(vec4(0.5, 0.5, 0.5, 1)); //vs_norm;
}

