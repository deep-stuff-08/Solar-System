#version 460 core

vec2 vertices[] = vec2[](
	vec2(1.0, 1.0),
	vec2(-1.0, 1.0),
	vec2(1.0, -1.0),
	vec2(-1.0, -1.0)
);

void main(void) {
	gl_Position = vec4(vertices[gl_VertexID], 0.0, 1.0);
}