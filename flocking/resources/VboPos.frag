#version 110
uniform vec3 eyePos;
uniform float numLights;
uniform float invNumLights;
uniform float invNumLightsHalf;
uniform sampler2D lightsTex;
uniform float att;

varying float leadership;
varying float crowd;
varying vec4 vVertex;
varying vec3 vNormal;
varying vec3 vLightDir;


vec3 PointLight( vec3 vertex, vec3 normal, vec3 eyeDirN, vec3 lightCenter, float lightRadius, vec3 color, float cutoff )
{
    vec3 lightDir	= lightCenter - vertex;
    float distance	= length( lightDir ) - lightRadius;
	distance *= distance;
    float d			= max( distance, 0.0 );
    lightDir		/= distance;
//    vec3 HV			= normalize( normalize( lightDir ) + eyeDirN );
	
    // calculate basic attenuation
    float denom = d/lightRadius + 1.0;
//	float denom = d + 1.0;
    float attenuation = 1.0 / ( denom*denom );
    
    // scale and bias attenuation such that:
    //   attenuation == 0 at extent of max influence
    //   attenuation == 1 when d == 0
    attenuation		= ( attenuation - cutoff ) / ( 1.0 - cutoff );
    attenuation		= max( attenuation, 0.0 );
    
    float diff		= max( dot( lightDir, normal ), 0.0 );
//	float NdotHV	= max( dot( normal, HV ), 0.0 );
//	float spec		= pow( NdotHV, 131.0 );
    return color * diff * attenuation;
}



void main()
{
	
	vec3 eyeDir			= eyePos - vVertex.xyz;
	vec3 eyeDirNorm		= normalize( eyeDir );
	float distFog		= max( 1.0 - length( eyeDir ) * 0.007, 0.0 );
	float depthFog		= 1.0 - ( 50.0 - vVertex.y )/150.0;
	 
	float ppEyeDiff		= max( dot( vNormal, vLightDir ), 0.0 );
	float ppSpec		= pow( ppEyeDiff, 20.0 );
	float ppFres		= pow( 1.0 - ppEyeDiff, 2.0 );
	
	
	
	vec3 baitLighting = vec3( 0.0 );
	float index = invNumLightsHalf;
	for( float i=0.0; i<numLights; i+=1.0 ){
		vec4 pos		= texture2D( lightsTex, vec2( index, 0.25 ) );
		vec4 color		= texture2D( lightsTex, vec2( index, 0.75 ) );
		float radius	= pos.a;
		baitLighting    += PointLight( vVertex.xyz, vNormal, eyeDirNorm, pos.rgb, 1.0, color.rgb, att );
		index			+= invNumLights;
	}
	
//	vec3 litRoomColor	= vec3( ppEyeDiff * 0.2 );// * ( vNormal.y * 0.5 + 0.5 );

	vec3 fogColor		= vec3( 15.0, 41.0, 42.0 )/255.0;
	vec3 blueish		= vec3( 0.035, 0.45, 0.4 );
	vec3 darkRoomColor	= baitLighting + ( crowd + ppEyeDiff * 0.2 + ppSpec ) * blueish;
	
	gl_FragColor.rgb	= mix( fogColor * depthFog, darkRoomColor, distFog );
	gl_FragColor.a		= 1.0;
}



