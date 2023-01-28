#version 450 core

out vec2 texCoords;

void main(void) {
	const vec4 vertex[] = vec4[](
		vec4(-1.0, -1.0, 0.5, 1.0),
		vec4(1.0, -1.0, 0.5, 1.0),
		vec4(-1.0, 1.0, 0.5, 1.0),
		vec4(1.0, 1.0, 0.5, 1.0)
	);

	gl_Position = vertex[gl_VertexID];
	texCoords = clamp(vertex[gl_VertexID].xy, 0.0, 1.0);
}