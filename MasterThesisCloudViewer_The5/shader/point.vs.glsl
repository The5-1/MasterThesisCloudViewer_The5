#version 330
  
layout(location = 0) in  vec3 vPosition; 
layout(location = 1) in  vec3 vNormal; 
layout(location = 2) in  vec3 vColor; 
layout(location = 3) in  float vRadius; 

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform vec3 col;
uniform vec3 viewPoint;
uniform vec3 lightPos;


uniform float glPointSize;

out vec4 viewNormal;
out vec4 viewPosition;
out vec3 color;
out vec4 positionFBO;
out vec3 lightVecV;

//#define BACKFACE_CULLING 0

//Pointsize
//#define PS_BASIC 0
//#define PS_CONSTANT 0
//#define PS_LION 0

void main() {
	mat4 normalMatrix = transpose(inverse( viewMatrix * modelMatrix));
	viewNormal =  normalMatrix * vec4(vNormal, 0.0); //homogenous coordinate = 0 for vectors

	//mat3 normalMatrix = inverse(transpose(mat3(modelMatrix[0].xyz, modelMatrix[1].xyz, modelMatrix[2].xyz)));
	//viewNormal =  vec4(normalize(normalMatrix * vNormal), 1);

	color = vColor;
		
	viewPosition = viewMatrix * modelMatrix * vec4(vPosition, 1.0);
	
	lightVecV = (viewMatrix * vec4(normalize(lightPos - vPosition),0.0)).xyz; //for vectors homogenous coordinate = 0
	//lightVecV = (viewMatrix * vec4(normalize(vec3(10.0,10.0,0.0)-vPosition),0.0)).xyz; //for vectors homogenous coordinate = 0

	//gl_PointSize = 15.0f;//*(1.0-viewPosition.z);
	positionFBO = modelMatrix * vec4(vPosition, 1.0);
	gl_Position = projMatrix * viewPosition;
	
	//Constant point size
	//gl_PointSize = 500.0f * vRadius * (1.0-gl_Position.z/gl_Position.w);
	gl_PointSize = glPointSize * (1.0 - gl_Position.z/gl_Position.w);
	//View-Dependant point size
	//gl_PointSize = 50.0f;

	/*
	float uPointSizeMultiplicator = 10.0f;
	float trans = 0.5 + max(length(viewPosition)-10.0, 0.0) / 30.0;
	vec4 p1 = projMatrix * viewPosition;
	vec4 p2 = projMatrix * (viewPosition + vec4(trans,trans,0.0,0.0));
	p1.xyz = p1.xyz / p1.w;
	p2.xyz = p2.xyz / p1.w;
	vec2 dist = p1.xy - p2.xy;
	float ps = length(dist) * 30.0;
	ps = max(3.0, ps);
	ps = ps * uPointSizeMultiplicator;
	gl_PointSize = ps;
	*/

	/* ******************************************************************
	Backface-Culling (gl_points dont have an implemented backface-cull)
	****************************************************************** */
	#ifdef BACKFACE_CULLING
	vec3 viewDirection = viewPoint - vPosition;
	if(dot(viewDirection, vNormal) < 0 ){
		gl_Position.w = 0.0;
	}
	#endif

	/* ******************************************************************
		Calculate tangent Vecotrs for the point
		-Use the Viewspace Normal (now the z-Coordinate represent ALWAYS the depth, its in the (-1,-1,-1) to (1,1,1) unit cube)
		-Now we can decide which is the best Tangent direction
	****************************************************************** */
	/*
	vec3 u, v;
	  if (abs(viewNormal.y) > abs(viewNormal.x))
	  {
		v = cross(viewNormal.xyz, vec3(1.0, 0.0, 0.0));
		u = cross(viewNormal.xyz, v);
	  }
	  else
	  {
		v = cross(vec3(0.0, 1.0, 0.0), viewNormal.xyz);
		u = cross(viewNormal.xyz, v);
	  }
	*/
}

