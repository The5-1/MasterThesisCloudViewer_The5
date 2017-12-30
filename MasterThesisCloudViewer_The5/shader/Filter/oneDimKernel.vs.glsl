/*
Source: https://github.com/mattdesl/lwjgl-basics/wiki/ShaderLesson5
*/
#version 330
layout(location = 0) in  vec3 vPosition;

out vec2 vTexCoord;
			
void main() { 
    vTexCoord = vPosition.xy * vec2(0.5) + vec2(0.5);
    gl_Position =  vec4(vPosition, 1.0);
}