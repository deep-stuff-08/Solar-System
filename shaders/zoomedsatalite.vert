#version 460 core

layout(location = 0)in vec4 vPosition;
layout(location = 2)in vec3 vNormal;
layout(location = 3)in vec2 vTexCoord;

layout(location = 0)uniform mat4 pMat;
layout(location = 2)uniform mat4 mvMat;
layout(location = 3)uniform vec3 lightPos;

out VS_OUT {
	vec3 N;
	vec3 L;
	vec2 texCoord;
} vs_out;

void main(void) {
	vec4 P = mvMat * vPosition;
	vs_out.N = mat3(mvMat) * vNormal;
	vs_out.L = lightPos - P.xyz;
	vs_out.texCoord = vTexCoord;
	gl_Position = pMat * P;
}