#version 330
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outPos;
layout(location = 3) out vec4 outDepth;

in vec2 tc;

uniform sampler2D texColor;
uniform sampler2D texNormal;
uniform sampler2D texPosition;
uniform sampler2D texDepth;

uniform vec3 lightVecV;

//Lighting
//#define PHONG 0
#define BLENDING 0
#define DEPTH_DIFFERENCE 0

void main(){
    vec4 dif = texture2D(texColor, tc);
	vec4 nor = texture2D(texNormal, tc);
	vec4 pos = texture2D(texPosition, tc);
	vec4 depth = texture2D(texDepth, tc);
	
	//Lighting
	#ifdef PHONG
		//float lightint = 1.0 / (length(pos - lightVecV)*length(pos - lightVecV));
		float lightint = 1.0;
		vec3 lightDir = normalize(lightVecV - pos.rgb);
		vec3 lightout = max(dot(nor.rgb, lightDir), 0.0) * dif.rgb * lightint;
		vec3 lightColor = vec3(1.0, 1.0, 1.0);
		outColor = vec4(lightColor * lightout, 1.0);
	#endif

	#ifdef BLENDING
		outColor = dif / dif.w;
		outNormal = nor / nor.w;
		outPos = pos / pos.w;
		gl_FragDepth = texture2D(texDepth, tc).r;

		/*
		outColor = vec4(texture2D(texColor, tc).rgb, 1.0);
		outNormal = vec4(texture2D(texNormal, tc).rgb, 1.0);
		outPos = vec4(texture2D(texPosition, tc).rgb, 1.0);
		outDepth = vec4(vec3(texture2D(texDepth, tc).r), 1.0);
		gl_FragDepth = outDepth.r;
		*/
	#endif
}
