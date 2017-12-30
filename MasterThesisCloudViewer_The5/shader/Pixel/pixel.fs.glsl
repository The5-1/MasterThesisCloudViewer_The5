/* GUIDES:
How to use glPoints: https://stackoverflow.com/questions/27098315/render-large-circular-points-in-modern-opengl
*/
#version 330
layout(location = 0) out vec4 outColor;

in vec3 color;

void main(){ 
	
	vec2 circCoord0 = gl_PointCoord;

	float alpha = min(1.0,(1.0-length(circCoord0*2.0-1.0))*4.0);

	if(alpha <= 0.0)
	{
		discard;
	}
	
	
	outColor = vec4(color,1.0);
	
}
