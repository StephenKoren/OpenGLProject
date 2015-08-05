#version 410

in vec2 vTexCoord;

out vec4 FragColour;

uniform sampler2D albedoTexture; //Base geometry colour info.
uniform sampler2D lightTexture; //Light color modifications to geometry.
uniform sampler2D fxTexture; //effects (particles) colour info.
uniform sampler2D shadowMap; //effects (particles) colour info.

uniform vec3 ambCol; //Ambient Light color

void main() {
	vec4 albedo = texture(albedoTexture, vTexCoord);
	vec4 light = texture(lightTexture, vTexCoord);
	vec4 fx = texture(fxTexture, vTexCoord);
	vec4 shadow = texture(shadowMap, vTexCoord);

	//fx objects are rendered independant of the other two, so they blend additively.
	FragColour = albedo * (light + vec4(ambCol, 0)) + fx;
}