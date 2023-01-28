#version 460 core

layout(binding = 0)uniform sampler2D sam;
layout(location = 3)uniform bool dofMode;

layout(location = 0)out vec4 vColor;

in VS_OUT {
	vec2 texCoord;
	float depth;
} fs_in;

void main(void) {
	vColor = texture(sam, fs_in.texCoord);
	if(dofMode) {
		vColor.a = fs_in.depth;
	} else {
		vColor.a = 1.0;
	}
}