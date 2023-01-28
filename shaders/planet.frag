#version 460 core

layout(binding = 0)uniform sampler2DArray sam;

layout(location = 0)out vec4 vColor;
layout(location = 1)out int objectID;

in VS_OUT {
	vec3 N;
	vec3 L;
	float depth;
	vec2 texCoord;
	flat int layer;
} fs_in;

void main(void) {
	vec3 N = normalize(fs_in.N);
	vec3 L = normalize(fs_in.L);
	if(fs_in.layer == 0) {
		vColor = texture(sam, vec3(fs_in.texCoord, fs_in.layer));
	} else {
		vColor = (0.05 + max(dot(N, L), 0.0)) * texture(sam, vec3(fs_in.texCoord, fs_in.layer));
	}
	vColor.a = fs_in.depth;
	objectID = fs_in.layer;
}