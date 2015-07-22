#version 410

out vec3 LightOutput;

uniform vec3 lightViewPos;
uniform vec3 lightCol;

uniform float lightRadius;

uniform sampler2D positionTexture;
uniform sampler2D normalTexture;
uniform sampler2D specularTexture;
/*
void main() {
	//texture coord
	vec2 texcoord = gl_FragCoord.xy / textureSize(positionTexture, 0).xy;

	//sample from textures
	vec4 positionSample = texture(positionTexture, texcoord);
	vec4 normalSample = texture(normalTexture, texcoord);
	vec4 specularSample = texture(specularTexture, texcoord);

	vec3 toLight = lightViewPos - positionSample.xyz;
	vec3 L = -normalize(toLight);
	vec3 N = normalize(normalSample.xyz);
	float falloff = 1 - min(1, (length(toLight) / lightRadius));

	float d = max(0.0, dot(-L, N));
	vec3 D = d * lightColor;
	
	vec3 E = -normalize(positionSample.xyz);
	vec3 R = reflect(L, N);
	float s = max(0, dot(R,E));
	s = pow(s, 1) * pow(falloff, 1);

	vec3 S = vec3(s) * lightColor;

	LightOutput = vec4(D + S,1) * falloff;
}
*/

void main() {
	//Numerical Constants
	float e = 2.71828182845904523536028747135f;
	float pi = 3.1415926535897932384626433832f;
	
	//texture coord
	vec2 texcoord = gl_FragCoord.xy / textureSize(positionTexture, 0).xy;

	//sample from textures
	vec4 positionSample = texture(positionTexture, texcoord);
	vec4 normalSample = texture(normalTexture, texcoord);
	vec4 specularSample = texture(specularTexture, texcoord);
	
	float roughness = normalSample.a; //Roughness is stored in normal alpha
	float fresnelScale = specularSample.a; //Likewise, fresnel scale is stored in the specular alpha.

	//Oren-Nayer Start
	vec3 toLight = lightViewPos - positionSample.xyz;
	vec3 L = normalize(toLight);
	vec3 N = normalize(normalSample.xyz);
	vec3 E = -normalize(positionSample.xyz);
	float falloff = 1 - min(1, (length(toLight) / lightRadius));

	float NdL = max(0.0f, dot(N, L));
	float NdE = max(0.0f, dot(N, E));

	//Roughness calculation
	float R2 = roughness * roughness;
	float RA = 1.0f - 0.5f * R2 / (R2 + 0.33f);
	float RB = 0.45f * R2 / (R2 + 0.09f);

	// CX = max(0, cos(r,i))
	vec3 lightProjected = normalize( L - N * NdL );
	vec3 viewProjected = normalize( E - N * NdE);
	float CX = max( 0.0f, dot( lightProjected, viewProjected ) );

	// DX = sin(alpha) * tan(beta)
	float alpha = sin( max( acos( NdE ), acos( NdL ) ) );
	float beta = tan( min( acos( NdE ), acos( NdL ) ) );
	float DX = alpha * beta;

	//Final Oren-Nayer Equation
	float OrenNayer = NdL * (RA + RB * CX * DX);

	//Cook-Torrance Start
	vec3 H = normalize( L + E ); // light and view half vector
	float NdH = max(0.0f, dot(N, H));
	float NdH2 = NdH * NdH;

	//Beckmann Distribution
	float exponent = -(1 - NdH2) / (NdH2 * R2);
	float D = pow(e, exponent) / (R2 * NdH2 * NdH2);

	//Fresnel Term
	float HdE = dot( H, E );
	float F = mix( pow( 1 - HdE, 5 ), 1, fresnelScale);

	//Geometric Attenutation
	float X = (2.0f * NdH) / dot( E, H );
	float G = min(1, min(X * NdE, X * NdL));

	//Final Cook-Torrance Equation
	float CookTorrance = max(0.0f, (F*G*D) / (NdE * pi));

	LightOutput = lightCol * (OrenNayer + (specularSample.xyz * CookTorrance)) * falloff;
}