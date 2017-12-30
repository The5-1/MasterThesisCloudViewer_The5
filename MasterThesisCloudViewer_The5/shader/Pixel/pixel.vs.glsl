#version 330
  
layout(location = 0) in  vec3 vPosition; 
layout(location = 1) in  vec3 vColor; 

uniform float glPointSize;

uniform mat4 projMatrix;
uniform mat4 viewMatrix;

out vec3 color;

void main() {
	color = vColor;
	//color = vec3(1.0, 0.0, 0.0);

	gl_Position = projMatrix * viewMatrix * vec4(vPosition, 1.0);

	gl_PointSize = glPointSize;
	//gl_PointSize = glPointSize * (1.0 - gl_Position.z/gl_Position.w);
}

