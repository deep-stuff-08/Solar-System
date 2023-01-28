#version 460 core

in VS_OUT {
	vec2 texCoords[11];
} fs_in;

layout(binding = 0)uniform sampler2D sam;

layout(location = 2)uniform float weights[11];

layout(location = 0)out vec4 color;

void main(void) {
	vec4 texSum = vec4(0.0);
	for(int i = 0; i < 11; i++) {
		texSum += texture(sam, fs_in.texCoords[i]) * weights[i];
	}
	color = texSum;
}