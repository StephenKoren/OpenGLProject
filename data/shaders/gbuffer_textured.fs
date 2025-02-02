#version 410

in vec4 fPosition;
in vec4 fNormal;
in vec2 fTexCoord;
in vec4 fColor;

layout(location = 0) out vec4 gPassAlbedo;
layout(location = 1) out vec4 gPassPosition;
layout(location = 2) out vec4 gPassNormal;
layout(location = 3) out vec4 gPassSpecular;

uniform sampler2D diffuse;
uniform sampler2D normal;
uniform sampler2D specular;

uniform float roughness;
uniform float fresnelScale;
uniform float textureScale;

uniform mat4 view;

void main() {
	gPassAlbedo = texture(diffuse, fTexCoord * 1 / textureScale) * fColor;
	gPassPosition = view * fPosition;
	gPassNormal = vec4((view * fNormal).xyz, roughness);
	gPassSpecular = vec4(texture(specular, fTexCoord).xyz, fresnelScale);
}