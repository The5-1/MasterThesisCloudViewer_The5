#version 330
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outPos;
//layout(location = 3) out vec4 outDepth;

in vec2 vTexCoord;

//declare uniforms
uniform sampler2D texColor;
uniform sampler2D texNormal;
uniform sampler2D texPosition;
uniform sampler2D texDepth;

uniform float resolutionWIDTH;
uniform float resolutionHEIGHT;
uniform float radius;
uniform vec2 dir;


vec3 filterTexture(sampler2D texture){
	//this will be our RGBA sum
	vec4 sum = vec4(0.0);
	
	//our original texcoord for this fragment
	vec2 tc = vTexCoord;
	
	//the amount to blur, i.e. how far off center to sample from 
	//1.0 -> blur by one pixel
	//2.0 -> blur by two pixels, etc.
	float blurWidth = radius / resolutionWIDTH; 
    float blurHeight = radius / resolutionHEIGHT; 

	//the direction of our blur
	//(1.0, 0.0) -> x-axis blur
	//(0.0, 1.0) -> y-axis blur
	float hstep = dir.x;
	float vstep = dir.y;
    
	//apply blurring, using a 9-tap filter with predefined gaussian weights

	sum += texture2D(texture, vec2(tc.x - 4.0*blurWidth*hstep, tc.y - 4.0*blurHeight*vstep)) * 0.0162162162;
	sum += texture2D(texture, vec2(tc.x - 3.0*blurWidth*hstep, tc.y - 3.0*blurHeight*vstep)) * 0.0540540541;
	sum += texture2D(texture, vec2(tc.x - 2.0*blurWidth*hstep, tc.y - 2.0*blurHeight*vstep)) * 0.1216216216;
	sum += texture2D(texture, vec2(tc.x - 1.0*blurWidth*hstep, tc.y - 1.0*blurHeight*vstep)) * 0.1945945946;
	
	sum += texture2D(texture, vec2(tc.x, tc.y)) * 0.2270270270;
	
	sum += texture2D(texture, vec2(tc.x + 1.0*blurWidth*hstep, tc.y + 1.0*blurHeight*vstep)) * 0.1945945946;
	sum += texture2D(texture, vec2(tc.x + 2.0*blurWidth*hstep, tc.y + 2.0*blurHeight*vstep)) * 0.1216216216;
	sum += texture2D(texture, vec2(tc.x + 3.0*blurWidth*hstep, tc.y + 3.0*blurHeight*vstep)) * 0.0540540541;
	sum += texture2D(texture, vec2(tc.x + 4.0*blurWidth*hstep, tc.y + 4.0*blurHeight*vstep)) * 0.0162162162;

	return sum.rgb;
}


vec3 filterTextureWithDepth(sampler2D colorTexture, sampler2D depthTexture){
	//this will be our RGBA sum
	vec4 sum = vec4(0.0);
	
	//our original texcoord for this fragment
	vec2 tc = vTexCoord;
	
	//the amount to blur, i.e. how far off center to sample from 
	//1.0 -> blur by one pixel
	//2.0 -> blur by two pixels, etc.
	float blurWidth = radius / resolutionWIDTH; 
    float blurHeight = radius / resolutionHEIGHT; 

	//the direction of our blur
	//(1.0, 0.0) -> x-axis blur
	//(0.0, 1.0) -> y-axis blur
	float hstep = dir.x;
	float vstep = dir.y;
    
	//apply blurring, using a 9-tap filter with predefined gaussian weights

	float currentDepth = texture2D(depthTexture, vec2(tc.x, tc.y)).r;

	sum += texture2D(colorTexture, vec2(tc.x - 4.0*blurWidth*hstep, tc.y - 4.0*blurHeight*vstep)) * 0.0162162162;
	sum += texture2D(colorTexture, vec2(tc.x - 3.0*blurWidth*hstep, tc.y - 3.0*blurHeight*vstep)) * 0.0540540541;
	sum += texture2D(colorTexture, vec2(tc.x - 2.0*blurWidth*hstep, tc.y - 2.0*blurHeight*vstep)) * 0.1216216216;
	sum += texture2D(colorTexture, vec2(tc.x - 1.0*blurWidth*hstep, tc.y - 1.0*blurHeight*vstep)) * 0.1945945946;
	
	sum += texture2D(colorTexture, vec2(tc.x, tc.y)) * 0.2270270270;
	
	sum += texture2D(colorTexture, vec2(tc.x + 1.0*blurWidth*hstep, tc.y + 1.0*blurHeight*vstep)) * 0.1945945946;
	sum += texture2D(colorTexture, vec2(tc.x + 2.0*blurWidth*hstep, tc.y + 2.0*blurHeight*vstep)) * 0.1216216216;
	sum += texture2D(colorTexture, vec2(tc.x + 3.0*blurWidth*hstep, tc.y + 3.0*blurHeight*vstep)) * 0.0540540541;
	sum += texture2D(colorTexture, vec2(tc.x + 4.0*blurWidth*hstep, tc.y + 4.0*blurHeight*vstep)) * 0.0162162162;

	return sum.rgb;
}

void main() {
	//Color
 	//outColor = vec4(texture2D(texColor, vTexCoord).rgb, 1.0);
	outColor = vec4( filterTextureWithDepth(texColor, texPosition), 1.0);
	//outColor = vec4( filterTexture(texColor), 1.0);
	//outColor = vec4(vec3(texture2D(texDepth, vTexCoord).r), 1.0); //Can NOT directly write depth into rgb buffer

	//Normal
    outNormal = vec4(texture2D(texNormal, vTexCoord).rgb, 1.0);
	outNormal = vec4( filterTexture(texNormal), 1.0);

	//Position
    outPos = vec4(texture2D(texPosition, vTexCoord).rgb, 1.0);
	outPos = vec4( filterTexture(texPosition), 1.0);

	//Depth
    //outDepth = vec4(vec3(texture2D(texDepth, vTexCoord).r), 1.0);
	//outDepth = vec4( filterTexture(texDepth), 1.0);
	
	gl_FragDepth = texture2D(texDepth, vTexCoord).r;
	//gl_FragDepth = filterTexture(texDepth).r;

}