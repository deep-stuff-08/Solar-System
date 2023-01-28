#version 460 core

layout(location = 0)uniform int samplesPerAxis;
layout(location = 1)uniform vec4 sphere[9];

#define EPSILON 0.000000001

out float scalarfield;

void main(void) {
	int posIn1D = gl_VertexID;
	ivec3 posIn3D = ivec3(posIn1D % samplesPerAxis, (posIn1D / samplesPerAxis) % samplesPerAxis, posIn1D / (samplesPerAxis * samplesPerAxis));
	vec3 currentPoint = vec3(-20.0) + posIn3D * (40.0 / (samplesPerAxis - 1));
	float sf = 0.0;
	for(int i = 0; i < 9; i++) {
		float dist = distance(currentPoint, sphere[i].xyz);
		sf += pow(sphere[i].w, 3) / pow(max(dist, EPSILON), 3);
	}
	scalarfield = sf;
}