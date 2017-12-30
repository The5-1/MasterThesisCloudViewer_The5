#version 330
  
layout(location = 0) in  vec3 vPosition; 
layout(location = 1) in vec3 vColor;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

out vec3 color;

void main() {
	color = vColor;
	//color = vec3(0.0, 0.0, 1.0);
	gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(vPosition, 1);
}

