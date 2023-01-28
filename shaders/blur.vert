#version 460 core

out VS_OUT {
	vec2 texCoords[11];
} vs_out;

layout(location = 0) uniform int winSize;
layout(location = 1) uniform ivec2 texFilter;

vec2 vertices[] = vec2[](
	vec2(1.0, 1.0),
	vec2(-1.0, 1.0),
	vec2(1.0, -1.0),
	vec2(-1.0, -1.0)
);

void main(void) {
	gl_Position = vec4(vertices[gl_VertexID], 0.0, 1.0);
	vec2 texCoord = vertices[gl_VertexID] * 0.5 + 0.5;
	float pixelOffset = 1.0 / float(winSize);
	for(int i = -5; i <= 5; i++) {
		vs_out.texCoords[i + 5] = texCoord + (vec2(pixelOffset * i) * texFilter);
	}
}