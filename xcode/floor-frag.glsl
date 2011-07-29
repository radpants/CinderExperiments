#version 110

uniform sampler2D texDiffuse;
uniform sampler2D texNormal;
uniform sampler2D texAO;
uniform vec3 lightDir;

varying vec3 normal;
varying vec3 position;

void main()
{
	vec2 texCoord			= vec2( gl_TexCoord[0].s, gl_TexCoord[0].t );
    vec2 minCoord           = vec2( mod(texCoord.x * 10.0, 1.0), mod(texCoord.y * 10.0, 1.0) );;
	vec3 diffuseSample		= texture2D( texDiffuse, texCoord ).rgb;
    vec3 detailSample       = texture2D( texDiffuse, minCoord ).rgb;
	vec3 normalSample		= ( texture2D( texNormal, texCoord ).rgb - vec3( 0.5, 0.5, 0.5 ) ) * 2.0;
    float aoSample           = texture2D( texAO, texCoord ).r;

	float water = 1.0;
	if( diffuseSample.b > diffuseSample.g && diffuseSample.b > diffuseSample.r ){
		water = 1.0;
	}

    diffuseSample = ( diffuseSample * 3.0 + detailSample ) / 4.0;

	vec3 ppNormal			= normalize( normal + normalSample );
	float ppDiffuse			= abs( dot( ppNormal, lightDir ) );
	float ppFresnel			= pow( ( 1.0 - ppDiffuse ), 3.0 );
	float ppSpecular		= pow( ppDiffuse, 10.0 ) * aoSample * detailSample.r;
	float ppSpecularBright	= pow( ppDiffuse, 10.0 ) * aoSample * detailSample.r;
    float shadows = 1.0 - ppSpecular;
    
    //if(shadows < 0.3) aoSample *= shadows;

	vec3 landFinal			= diffuseSample + ppSpecularBright;
	vec3 oceanFinal			= diffuseSample * ppSpecular * ppSpecularBright * ppFresnel * 15.0;

	float r					= ( 1.0 - ppNormal.r ) * 0.5;
	gl_FragColor.rgb		= aoSample * (oceanFinal + landFinal);// + vec3( r*r, r * 0.25, 0 ) * oceanValue;
	gl_FragColor.a			= 1.0;
}

