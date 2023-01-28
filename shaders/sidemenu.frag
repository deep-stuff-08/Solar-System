#version 460 core

in VS_OUT {
	vec2 texCoord;
} fs_in;

layout(binding = 0)uniform sampler2D sam;

layout(location = 0)uniform bool enableBorder;

layout(location = 0)out vec4 vColor;

void main(void) {
	if(enableBorder && (fs_in.texCoord.x < 0.02 || fs_in.texCoord.x > 0.98 ||fs_in.texCoord.y < 0.02 || fs_in.texCoord.y > 0.98)) {
		vColor = vec4(1.0);
	} else {
		vColor = texture(sam, fs_in.texCoord);
	}
}