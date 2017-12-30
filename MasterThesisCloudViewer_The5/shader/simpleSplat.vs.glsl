//http://www.mbroecker.com/research_pbr.html

#version 330
  
layout(location = 0) in  vec3 vPosition; 
layout(location = 1) in  vec3 vNormal; 
layout(location = 2) in  vec3 vColor; 

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform vec3 col;

 out VertexData 
{
	vec3  color;
	vec3  normal;
	float radius;
} vertexOut;

void main() {
	gl_Position = vec4(vPosition, 1.0);

	//vertexOut.color = col;
	vertexOut.color = vColor;
	
	vertexOut.normal = normalize(vNormal);
	vertexOut.radius = 0.06;
}

