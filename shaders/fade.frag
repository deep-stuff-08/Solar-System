#version 460 core

layout(location = 0) uniform float fadeAlpha;

layout(location = 0) out vec4 color;

void main(void) {
	color = vec4(0.0, 0.0, 0.0, fadeAlpha);
}