#version 460 core

layout(location = 0)in vec2 vPosition;
layout(location = 3)in vec2 vTexcoord;

layout(location = 2)uniform mat4 mMat;

out VS_OUT {
	vec2 texCoord;
} vs_out;

void main(void) {
	vs_out.texCoord = vTexcoord;
	gl_Position = mMat * vec4(vPosition, 0.0, 1.0);
}