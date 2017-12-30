
#version 330
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outPos;
layout(location = 3) out vec4 outDepth;

uniform float depthEpsilonOffset;
uniform sampler1D filter_kernel;
uniform sampler2D texDepth;

in vec4 viewNormal;
in vec4 viewPosition;
in vec3 color;
in vec4 positionFBO;

in vec3 coordinateGridPos;

#define AFFINE_PROJECTED 0
#define GAUSS_ALPHA 0


void main(){ 
	//outNormal = vec4(0.5 * viewNormal.xyz + vec3(0.5), 1.0);
	//outPos = vec4(positionFBO.xyz/positionFBO.w, 1.0);
	//outNormal = vec4( vec3(texture(texDepth, 0.5 * gl_FragCoord.xy +0.5).r) , 1.0);
	//outPos = vec4( vec3(texture(texDepth, gl_FragCoord.xy).r), 1.0);

	#ifdef AFFINE_PROJECTED
		vec2 circCoord = 2.0 * gl_PointCoord - vec2(1.0, 1.0); //Maps to [-1, 1]

		//IMPORTANT: Changed one of them to positive -> seems to fix the error that they are displaced (doesnt matter which is positive for teapot/sphere)
		float delta_z = (+ ((viewNormal.x ) / (viewNormal.z )) * circCoord.x - ((viewNormal.y ) / (viewNormal.z )) * circCoord.y);

		float maxRadius = 1.0;
		float currentRadius = length( vec3(circCoord.x, circCoord.y, delta_z) );

		//float previous_buffered_epsillon_depth = 1.0; //TODO here we need a sampler to get the previous depth with epsillon offset!
		//float new_fragment_depth = gl_FragCoord.z;
		//float delta_depth_for_blending = max(0.0,new_fragment_depth-previous_buffered_epsillon_depth);

		if(currentRadius > maxRadius)
		{
			//outColor = abs(vec4(1.0, 0.0, 0.0, 1.0));
			discard;
		}
		else{
			float alpha = 1.0;
			#ifdef GAUSS_ALPHA
				alpha = texture(filter_kernel, currentRadius).r;
			#endif
			//outColor = vec4(vec3(texture(filter_kernel, currentRadius).r), alpha);
			//outColor = vec4(color, alpha);
		}

		//Update depth
		float newDepth = gl_FragCoord.z + (pow(currentRadius, 2)) * gl_FragCoord.w + depthEpsilonOffset;

		outDepth = vec4(vec3(newDepth), 1.0);
		outColor = vec4(vec3(newDepth), 1.0);
		outNormal = vec4(vec3(newDepth), 1.0);
		outPos= vec4(vec3(newDepth), 1.0);
		gl_FragDepth = newDepth; 
	#endif
}
