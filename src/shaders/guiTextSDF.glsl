
#shader VERTEX


#version 330 core

layout (location = 0) in vec3 pos_in;
layout (location = 1) in vec2 tex_in;
layout (location = 2) in vec4 color_in;

uniform mat4 mProj;
uniform mat4 mModel;


out vec2 texCoord;
out vec4 color;

void main() {
// 	gl_Position = (mProj * mView * mModel) * vec4(pos_in.xyz, 1.0);

	texCoord = tex_in;
	color = color_in;
	gl_Position = (mProj * mModel) * vec4(pos_in.xyz, 1.0);
}




#shader FRAGMENT

#version 330


// fragment shader
uniform sampler2D fontTex;
uniform float smoothLower;
uniform float smoothUpper;
uniform float smoothLimit;

in vec2 texCoord;
in vec4 color;



void main(void) {

	float d = texture2D(fontTex, texCoord).r;
	
	float a;
	
	d = 1 - d;

	a = smoothstep(smoothLower, smoothUpper, abs(d));

	if(a < smoothLimit) discard;
	
	gl_FragColor = vec4(color.abg, a); 

}

