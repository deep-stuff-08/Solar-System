#version 460 core

layout(location = 0)in vec2 vPosition;
layout(location = 3)in vec2 vTexCoord;

layout(location = 0)uniform mat4 pMat;
layout(location = 2)uniform mat4 mvMat;

out VS_OUT {
	vec2 texCoord;
	float depth;
} vs_out;

void main(void) {
	vs_out.texCoord = vTexCoord;
	vec4 P = mvMat * vec4(vPosition.x, 0.0, vPosition.y, 1.0);
	vs_out.depth = P.z;
	gl_Position = pMat * P;
}