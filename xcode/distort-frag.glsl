#version 110

uniform sampler2D texDiffuse;
uniform sampler2D texNormal;
uniform float distortPower;

varying vec3 normal;
varying vec3 position;

void main(){
	
	
	vec2 texCoord = vec2( gl_TexCoord[0].s, gl_TexCoord[0].t );
	float d = distance( texCoord, vec2(0.5, 0.5)  );
	d= d*d;
	
	vec3 normal = texture2D( texNormal, texCoord * 0.3).rgb;
	texCoord += (vec2(normal.r, normal.g) + vec2(-0.5, -0.5)) * distortPower * d;
	float g = texture2D( texDiffuse, texCoord).g;
	texCoord.x += distortPower * d;
	float r = texture2D( texDiffuse, texCoord ).r;
	texCoord.x += distortPower * d;
	float b = texture2D( texDiffuse, texCoord ).b;
	
//	vec3 color = texture2D( texDiffuse, texCoord ).rgb;
	vec3 color = vec3(r,g,b);

	gl_FragColor.rgb = color;
	gl_FragColor.a = 1.0;
}

