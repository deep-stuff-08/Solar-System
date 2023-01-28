#version 460 core

layout(location = 0)in vec4 vPosition;
layout(location = 2)in vec3 vNormal;
layout(location = 3)in vec2 vTexCoord;

layout(location = 0)uniform mat4 pMat;
layout(location = 1)uniform mat4 vMat;
layout(location = 2)uniform mat4 mMat[9];
layout(location = 11)uniform float lightZ;

out VS_OUT {
	vec3 N;
	vec3 L;
	float depth;
	vec2 texCoord;
	int layer;
} vs_out;

void main(void) {
	mat4 mvMat = vMat * mMat[gl_InstanceID];
	vec4 P = mvMat * vPosition;
	vs_out.N = mat3(mvMat) * vNormal;
	vs_out.L = vec3(0.0, 0.0, -lightZ) - P.xyz;
	vs_out.texCoord = vTexCoord;
	vs_out.layer = gl_InstanceID;
	vs_out.depth = -P.z;
	gl_Position = pMat * P;
}