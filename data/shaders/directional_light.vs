#version 410

layout(location = 0) in vec3 Position;
layout(location = 1) in vec4 Normal;

void main() {
	gl_Position = vec4(Position, 1);
}