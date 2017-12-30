/* GUIDES:
How to use glPoints: https://stackoverflow.com/questions/27098315/render-large-circular-points-in-modern-opengl
*/
#version 330
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outPos;

uniform float nearPlane;
uniform float farPlane;
uniform bool depthToPosTexture;

in vec4 viewNormal;
in vec4 viewPosition;
in vec3 color;
in vec4 positionFBO;

in vec3 lightVecV;

//Render type (only 1 may be active!!)
//#define SIMPLE_POINT 0
#define AFFINE_PROJECTED 0
//#define POTREE 0


//Lighting
//#define PHONG 0

void main(){ 
	/* ******************************************************************
	Fill FBO
	****************************************************************** */
	outNormal = vec4(0.5 * viewNormal.xyz + vec3(0.5), 1.0);
	outPos = vec4(positionFBO.xyz/positionFBO.w, 1.0);
	//outPos = vec4(viewPosition.xyz/viewPosition.w, 1.0);
	

	/* ******************************************************************
		Paint simple Point
	****************************************************************** */
	#ifdef SIMPLE_POINT
		vec2 circCoord0 = gl_PointCoord;
		float alpha = min(1.0,(1.0-length(circCoord0*2.0-1.0))*4.0);
		if(alpha <= 0.0)
		{
			discard;
		}
		
		//vec2 circCoord = 2.0 * gl_PointCoord - 1.0;
		
		//if (dot(circCoord, circCoord) > 1.0) {
		//	discard;
		//}
		
		
		//outColor = abs(vec4(vec3(circCoord.s, circCoord.t, 0.0)*alpha, 1.0));
		//outColor = vec4(1.0)*alpha;
		outColor = vec4(color,1.0);

		
		//outColor = abs(vec4(1.0, 0.0, 0.0, 1.0));
		//outColor = vec4(alpha, 1);
	#endif

	/* ******************************************************************
		Affinely Projected Point Sprites (Page 278)
	****************************************************************** */
	#ifdef AFFINE_PROJECTED
		vec2 circCoord = 2.0 * gl_PointCoord - vec2(1.0, 1.0); //Maps to [-1, 1]

		//IMPORTANT: Changed one of them to positive -> seems to fix the error that they are displaced (doesnt matter which is positive for teapot/sphere)
		float delta_z = (+ ((viewNormal.x ) / (viewNormal.z )) * circCoord.x - ((viewNormal.y ) / (viewNormal.z )) * circCoord.y);

		float maxRadius = 1.0;
		float currentRadius = length( vec3(circCoord.x, circCoord.y, delta_z) );
		if(currentRadius > maxRadius)
		{
			//outColor = abs(vec4(1.0, 0.0, 0.0, 1.0));
			discard;
		}
		else if(currentRadius >= 0.9 * maxRadius)
		{
			outColor = vec4(0.0, 0.0, 0.0, 1.0);

			#ifdef PHONG
			outColor = vec4(0.0, 0.0, 0.0, 1.0) * vec4(vec3(pow(max(0.0,dot(viewNormal.xyz, lightVecV)),4.0)),1.0);
			#endif
		}
		else{
			outColor = vec4(color, 1.0);

			#ifdef PHONG
			outColor = vec4(color, 1.0) * vec4(vec3(pow(max(0.0,dot(viewNormal.xyz, lightVecV)),4.0)),1.0);
			#endif
		}

		

		//Update depth
		gl_FragDepth = gl_FragCoord.z + (pow(currentRadius, 2.0)) * gl_FragCoord.w ; 
		if(depthToPosTexture){
			outPos = vec4(vec3( gl_FragCoord.z + (pow(currentRadius, 2.0)) * gl_FragCoord.w  ), 1.0);
		}
	#endif

	/* ******************************************************************
	Potree example
	****************************************************************** */
	#ifdef POTREE
		float a = pow(2.0*(gl_PointCoord.x - 0.5), 2.0);           
		float b = pow(2.0*(gl_PointCoord.y - 0.5), 2.0);           
		float c = 1.0 - (a + b);                                   
																
		if(c < 0.0){                                               
			discard;                                               
		}
		else if(c < 0.2){
			outColor = abs(vec4(0.0, 0.0, 0.0, 1.0));
		}                                                       
		else{														
			outColor = vec4(1.0, 0.0, 0.0, 1.0);  
		} 
	#endif


	/* ******************************************************************
	Recalc depth
	****************************************************************** */
	//gl_FragDepth = (1.0 / ( gl_FragCoord.z + delta_z )) * (farPlane * nearPlane) / (farPlane - nearPlane) + (farPlane) / (farPlane - nearPlane);

	/* ******************************************************************
		Why does this half the circles?
	****************************************************************** */
	//gl_FragDepth = 1.0 / ( gl_FragCoord.z, delta_z ) * (farPlane * nearPlane) / (farPlane - nearPlane) + (farPlane) / (farPlane - nearPlane);
	
}
