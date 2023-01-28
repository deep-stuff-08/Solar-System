#version 460 core

layout(location = 1)uniform mat4 vMat;

out VS_OUT {
	vec3 texCoord;
	float depth;
} vs_out;

void main(void) {
	vec3[4] vertices = vec3[4](
		vec3(-1.0, -1.0, 1.0),
		vec3(1.0, -1.0, 1.0),
		vec3(-1.0, 1.0, 1.0),
		vec3(1.0, 1.0, 1.0)
	);
	vs_out.texCoord = mat3(vMat) * vertices[gl_VertexID];
	vs_out.depth = vs_out.texCoord.z;
	gl_Position = vec4(vertices[gl_VertexID], 1.0);
}