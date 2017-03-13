#version 330 core

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 tPosition;
layout(location = 2) in vec3 nPosition;

uniform int vert0frag1;
uniform int off0on1;

out  vec3 fN;
out  vec3 fE;
out  vec3 fL;
out vec4 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec4 lightPos;
uniform vec4 modelColor;
uniform vec4 pos;

uniform vec4 AmbientProduct;
uniform vec4 DiffuseProduct;
uniform vec4 SpecularProduct;

uniform vec4 LightPosition;
uniform float Shininess;


out vec2 textures;

void main()
{

	if(vert0frag1 == 1) {
		fN = nPosition;
		fE = (view * model * (vec4(vPosition ,1) + pos)).xyz;
		fL = vPosition.xyz - lightPos.xyz;    

		gl_Position =  projection * view * model * (vec4(vPosition ,1) + pos);

		textures = tPosition;
	}
	else{
	

		textures = tPosition;
		gl_Position =  projection * view * model * (vec4(vPosition ,1) + pos);


		vec3 N = normalize(nPosition);
		vec3 E = normalize((view * model * (vec4(vPosition ,1) + pos)).xyz);
		vec3 L = normalize(vPosition.xyz - lightPos.xyz);

		vec3 H = normalize( L + E );
    
		vec4 ambient = AmbientProduct;

		float Kd = max(dot(L, N), 0.0);
		vec4 diffuse = Kd*DiffuseProduct;
    
		float Ks = pow(max(dot(N, H), 0.0), Shininess);
		vec4 specular = Ks*SpecularProduct;

		color = ambient + diffuse +specular ;

		color.a = 1.0;

	}

}
