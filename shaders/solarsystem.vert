#version 460 core

out VS_OUT {
	vec2 texCoord;
} vs_out;

vec2 vertices[] = vec2[](
	vec2(1.0, 1.0),
	vec2(-1.0, 1.0),
	vec2(1.0, -1.0),
	vec2(-1.0, -1.0)
);
	
void main(void) {
	gl_Position = vec4(vertices[gl_VertexID], 0.0, 1.0);
	vs_out.texCoord = clamp(vertices[gl_VertexID], 0.0, 1.0);
}