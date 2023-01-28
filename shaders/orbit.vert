#version 460 core

layout(location = 0)in vec2 vPosition;

layout(location = 0)uniform mat4 pMat;
layout(location = 1)uniform mat4 vMat;
layout(location = 2)uniform float mScale[8];

void main(void) {
	gl_Position = pMat * vMat * vec4(vPosition.x * mScale[gl_InstanceID], 0.0, vPosition.y * mScale[gl_InstanceID], 1.0);
}