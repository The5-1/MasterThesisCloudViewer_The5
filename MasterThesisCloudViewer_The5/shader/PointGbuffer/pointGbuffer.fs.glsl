/* GUIDES:
How to use glPoints: https://stackoverflow.com/questions/27098315/render-large-circular-points-in-modern-opengl
*/
#version 330
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outPos;
layout(location = 3) out vec4 outDepth;

uniform float depthEpsilonOffset;
uniform sampler1D filter_kernel;

in vec4 viewNormal;
in vec4 viewPosition;
in vec3 color;
in vec4 positionFBO;
in float beta;

in vec3 coordinateGridPos;

//Render type (only 1 may be active!!)
//#define SIMPLE_POINT 0
#define AFFINE_PROJECTED 0
#define GAUSS_ALPHA 0
#define SHOW_COORDINATE_GRID 0
//#define HARDWARE_ORIENTED_POINT_BASED_RENDERING_OF_COMPLEX_SCENES 0//page 6, http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.18.6803&rep=rep1&type=pdf

void main(){ 
	/* ******************************************************************
	Fill FBO
	****************************************************************** */
	outNormal = vec4(0.5 * viewNormal.xyz + vec3(0.5), 1.0);
	outPos = vec4(positionFBO.xyz/positionFBO.w, 1.0);

	/* ******************************************************************
		Affinely Projected Point Sprites (Page 278)
	****************************************************************** */
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
			outColor = vec4(color, alpha);
		}

		//Update depth
		float newDepth = gl_FragCoord.z + (pow(currentRadius, 2)) * gl_FragCoord.w + depthEpsilonOffset;
		// conversion into NDC [-1,1]
		//float n = 1.0;
		//float f = 500.0;
		//float zndc =  gl_FragCoord.z * 2.0 - 1.0;
		//float zeye = 2*f*n / (zndc*(f-n)-(f+n));
		//Near: 1, far 500
		//float newDepth = 1.0 / (- (0.5 * delta_z + 0.5) - (0.5 * gl_FragCoord.z + 0.5)) * 1.002 + 1.002; //Am besten, macht aber mathematisch keinen sinn
		//float newDepth = 1.0 / (- (2.0 * delta_z - 1.0)- (2.0 * gl_FragCoord.z - 1.0)) * 1.002 + 1.002;
		//float newDepth = 1.0 / (- delta_z - gl_FragCoord.z) * 1.002 + 1.002; //Am besten
		//float newDepth = 1.0 / (+ delta_z - gl_FragCoord.z) * 1.002 + 1.002;
		//Near: 5, far 500
		//float newDepth = 1.0 / (+ delta_z - gl_FragCoord.z) * 5.0505 + 1.0101;
		//float newDepth = 1.0 / (- delta_z - gl_FragCoord.z) * 5.0505 + 1.0101;

		outDepth = vec4(vec3(newDepth), 1.0);
		//outPos = vec4(vec3(newDepth), 1.0);
		gl_FragDepth = newDepth; 
	#endif

	/* ******************************************************************
		page 6, http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.18.6803&rep=rep1&type=pdf
	****************************************************************** */
	#ifdef HARDWARE_ORIENTED_POINT_BASED_RENDERING_OF_COMPLEX_SCENES
		//gl_PointCoord = 0.5 * gl_PointCoord;
		vec2 circCoord = 2.0 * gl_PointCoord - vec2(1.0, 1.0); //Maps to [-1, 1]

		float delta_z = (+ ((viewNormal.x ) / (viewNormal.z )) * circCoord.x - ((viewNormal.y ) / (viewNormal.z )) * circCoord.y);

		float maxRadius = 1.0;
		float currentRadius = length( vec3(circCoord.x, circCoord.y, delta_z) );

		if(currentRadius > maxRadius)
		{
			discard;
		}
		else{
			float alpha = 1.0;
			#ifdef GAUSS_ALPHA
				alpha = texture(filter_kernel, currentRadius).r;
			#endif
			outColor = vec4(vec3(beta), alpha);
		}

		float newDepth = gl_FragCoord.z + (pow(currentRadius, 2)) * gl_FragCoord.w + depthEpsilonOffset;

		outDepth = vec4(vec3(newDepth), 1.0);
		gl_FragDepth = newDepth; 
		
 	#endif


	/* ******************************************************************
		Simple Points
	****************************************************************** */
	#ifdef SIMPLE_POINT
	vec2 circCoord0 =  2.0 * gl_PointCoord - vec2(1.0, 1.0);;
	float alpha = circCoord0.x * circCoord0.x + circCoord0.y * circCoord0.y; 
	if(alpha > 1.0)
	{
		discard;
	}

	outColor = vec4(color, 1.0);
	
	float newDepth = gl_FragCoord.z + (pow(alpha, 2.0)) * gl_FragCoord.w + depthEpsilonOffset;
	outDepth = vec4(vec3(newDepth), 1.0);
	gl_FragDepth = newDepth; 

	#endif

	#if SHOW_COORDINATE_GRID == 1
		outColor = vec4(vec3(fract(coordinateGridPos)*10.0-9-0),1.0);
	#endif
}
