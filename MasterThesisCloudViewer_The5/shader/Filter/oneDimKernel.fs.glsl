#version 330
layout(location = 0) out vec4 outColor;
//layout(location = 3) out vec4 outDepth;

in vec2 vTexCoord;
uniform sampler1D filter_kernel;

void main() {
	//Color
	outColor = vec4( vec3(texture(filter_kernel, vTexCoord.x).r), 1.0);
}