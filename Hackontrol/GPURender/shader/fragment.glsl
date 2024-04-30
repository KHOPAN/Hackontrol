#version 330 core

layout(location = 0) out vec4 color;

uniform float inputTime;
uniform vec2 inputSize;

void main() {
	color = vec4(abs(sin(inputTime)), abs(sin(inputSize.x)), abs(sin(inputSize.y)), 1.0);
}
