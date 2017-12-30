#version 330
layout(location = 0)  out vec4 out0; // color 

uniform vec3 lightDir;

in vec3 color;
in vec4 camPos;
in vec3 normal;
in vec4 pos_ec;

void main(){ 

	vec3 N = normalize(normal);
	vec3 L = normalize(lightDir);   
	vec3 E = normalize(-pos_ec.xyz/pos_ec.w); // we are in Eye Coordinates, so EyePos is (0,0,0)  
	vec3 R = normalize(-reflect(L,N));  

	//calculate Ambient Term:  
	vec4 Iamb = vec4(color, 1.0f); 
	
	//calculate Diffuse Term:  
	vec4 diffColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	vec4 Idiff = diffColor * max(dot(N,L), 0.0);
	Idiff = clamp(Idiff, 0.0, 1.0);     

	// calculate Specular Term:
	float shininess = 5.0;
	vec4 specColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	vec4 Ispec = specColor * pow(max(dot(R,E),0.0),12 * shininess);
	Ispec = clamp(Ispec, 0.0, 1.0); 

	// write Total Color:  
	out0 = Iamb + Idiff + Ispec;

	//float lightint = 0.5f;
	//vec3 lightout = max(dot(normalize(normal),lightDir),0) * color * lightint;
	//out0 = vec4(lightout, 1);
}
