

#shader VERTEX


#version 330


layout (location = 0) in vec3 pos;


void main() {
	gl_Position = vec4(pos, 1.0);
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

// forward matrices
uniform mat4 mWorldView;
uniform mat4 mViewProj;

// inverse matrices
uniform mat4 mViewWorld;
uniform mat4 mProjView;

uniform vec2 resolution;

out vec4 FragColor;

void main() {
	vec2 tex = gl_FragCoord.xy / resolution.xy;
	vec3 normal = texture(sNormals, tex).xyz;
	
	if(debugMode == 0) {
		// normal rendering
		
		// reconstruct world coordinates
		vec2 screenCoord = gl_FragCoord.xy / resolution.xy;
		
		float depth = texture(sDepth, screenCoord).r;
		if (depth > 0.99999) {
			discard; // probably shouldn't be here
		}
		
		float ndc_depth = depth * 2.0 - 1.0;
		
		mat4 invVP = inverse(mViewProj * mWorldView);
		
		vec4 tmppos = invVP * vec4(screenCoord * 2.0 - 1.0, ndc_depth, 1.0);
		vec3 pos = tmppos.xyz / tmppos.w;
		// pos is in world coordinates now
		
		vec3 viewpos = (inverse(mViewProj * mWorldView) * vec4(0,0,0,1)).xyz;
		
		vec3 viewdir = normalize(viewpos - pos);
		vec3 sundir = normalize(vec3(3,3,20));
		
		float lambertian = max(dot(sundir, normal), 0.0);
		
		vec3 halfDir = normalize(sundir + viewdir);
		float specAngle = max(dot(halfDir, normal), 0.0);
		float specular = pow(specAngle, 16);

		vec3 diffuseColor = texture(sDiffuse, tex).rgb;
		vec3 specColor = vec3(0,0,0);//normalize(vec3(1,1,1));

		vec3 ambient = vec3(0.1,0.1,0.1);
		FragColor = vec4(ambient + lambertian * diffuseColor+ specular * specColor, 1.0);

	}
	else if(debugMode == 1) {
		// diffuse
		FragColor = vec4(texture(sDiffuse, tex).rgb,  1.0);
	}
	else if(debugMode == 2) {
		// normals
		FragColor = vec4(texture(sNormals, tex).rgb + vec3(.5,0,.1),  1.0);
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
