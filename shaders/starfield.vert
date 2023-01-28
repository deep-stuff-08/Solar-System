#version 460 core

layout(location = 0)in vec4 vPosition;
layout(location = 1)in vec4 vColor;

out VS_OUT {
	vec4 color;
	int opLocation;
} vs_out;

void main(void) {
	vs_out.color = vColor;
	vs_out.opLocation = gl_VertexID % 6;
	vec4 pos = vPosition;
	gl_PointSize = pos.z;
	pos.z = pos.z / 13.0;
	gl_Position = pos;
}