#version 330
  
layout(location = 0) in  vec3 vPosition; 
layout(location = 1) in  vec3 vColor; 
layout(location = 2) in  vec3 vNormal; 

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform vec3 col;

out vec3 color;
out vec4 viewCamPos;
out vec3 normal;
out vec4 pos_ec;

void main() {
	viewCamPos = viewMatrix * modelMatrix * vec4(vec3(0.0f, 0.0f, 0.0f), 1.0f);
	color = col;
	
	mat3 normalMatrix = transpose(inverse(mat3(viewMatrix * modelMatrix)));
	normal = normalMatrix * vNormal.xyz;
	pos_ec = viewMatrix * modelMatrix * vec4(vPosition, 1);
	
	gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(vPosition, 1);
}

