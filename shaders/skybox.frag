#version 460 core

in VS_OUT {
	vec3 texCoord;
	float depth;
} fs_in;

layout(binding = 0)uniform samplerCube cubeMap;
layout(location = 0)out vec4 color;

void main(void) {
	color = texture(cubeMap, fs_in.texCoord);
	color.a = -10.0;
}