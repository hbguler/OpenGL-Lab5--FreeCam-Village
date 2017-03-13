#version 330 core

uniform int vert0frag1;
uniform int off0on1;

in vec2 textures;
in  vec3 fN;
in  vec3 fL;
in  vec3 fE;
in vec4 color;
out vec4 fcolor;

uniform vec4 AmbientProduct;
uniform vec4 DiffuseProduct;
uniform vec4 SpecularProduct;

uniform vec4 LightPosition;
uniform float Shininess;

uniform sampler2D myTextureSampler;

void main() 
{ 

	if(vert0frag1 == 1) {

		vec3 N = normalize(fN);
		vec3 E = normalize(fE);
		vec3 L = normalize(fL);

		vec3 H = normalize( L + E );
    
		vec4 ambient = AmbientProduct;

		float Kd = max(dot(L, N), 0.0);
		vec4 diffuse = Kd*DiffuseProduct;
    
		float Ks = pow(max(dot(N, H), 0.0), Shininess);
		vec4 specular = Ks*SpecularProduct;

		vec4 tex = texture( myTextureSampler, textures );

		if(off0on1 == 1){
			fcolor = (ambient * tex) + (diffuse * tex) + (specular * tex) ;
		}
		else {
			fcolor = tex;
		}

		fcolor.a = 1.0;
	}

	else{

		vec4 tex = texture( myTextureSampler, textures );

		if(off0on1 == 1) {
			fcolor = color* tex;	
		}
		else {
			fcolor = tex ;
		}

		fcolor.a = 1.0;

		

	}

} 

