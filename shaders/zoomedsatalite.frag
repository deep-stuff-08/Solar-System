#version 460 core

layout(binding = 0)uniform sampler2DArray sam;

layout(location = 0)out vec4 vColor;
layout(location = 4)uniform int layer;

in VS_OUT {
	vec3 N;
	vec3 L;
	vec2 texCoord;
} fs_in;

void main(void) {
	vec3 N = normalize(fs_in.N);
	vec3 L = normalize(fs_in.L);
	if(layer == 0) {
		vColor = texture(sam, vec3(fs_in.texCoord, layer));
	} else {
		vColor = (0.1 + max(dot(N, L), 0.0)) * texture(sam, vec3(fs_in.texCoord, layer));
	}
	vColor.a = 1.0;
}