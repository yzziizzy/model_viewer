

#shader VERTEX


#version 330


layout (location = 0) in vec3 pos;

uniform mat4 world;

void main() {
	gl_Position = world * vec4(pos, 1.0);
}







#shader FRAGMENT


#version 330


uniform sampler2D sDiffuse;
uniform sampler2D sNormals;
uniform sampler2D sDepth;
uniform sampler2D sSelection;
uniform sampler2D sLighting;

uniform int debugMode;
uniform vec2 clipPlanes;

uniform vec3 sunNormal;

uniform vec2 resolution;

out vec4 FragColor;

void main() {
	vec2 tex = gl_FragCoord.xy / resolution.xy;
	
	if(debugMode == 0) {
		// normal rendering
		vec3 d = texture(sDiffuse, tex).rgb;
		vec3 amb = d * vec3(.2,.2,.2) * 2.9;
		vec3 sun = d * vec3(1.1, 1.0, .9) * 1.2* dot(sunNormal, texture(sNormals, tex).xyz);
		FragColor = vec4(amb + sun,  1.0);

	}
	else if(debugMode == 1) {
		// diffuse
		FragColor = vec4(texture(sDiffuse, tex).rgb,  1.0);
	}
	else if(debugMode == 2) {
		// normals
		FragColor = vec4(texture(sNormals, tex).rgb,  1.0);
	}
	else if(debugMode == 3) {
		// depth
		FragColor = vec4(texture(sDepth, tex).rrr,  1.0);
	}
	else if(debugMode == 4) {
		// selection buffer
		FragColor = vec4(texture(sSelection, tex).rgb,  1.0);
	}
	else if(debugMode == 5) {
		// lighting buffer
		FragColor = vec4(texture(sLighting, tex).rgb,  1.0);
	}

//	FragColor = vec4(texture(sNormals, tex).rgb,  1.0);
}
