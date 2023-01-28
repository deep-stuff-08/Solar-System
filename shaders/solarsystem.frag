#version 460 core

in VS_OUT {
	vec2 texCoord;
} fs_in;

layout(binding = 0) uniform sampler2D sam;

layout(location = 0) out vec4 color;

void main(void) {
	color = texture(sam, fs_in.texCoord);
}