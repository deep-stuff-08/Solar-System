#version 460 core

layout(location = 0)uniform int cellsPerAxis;
layout(location = 1)uniform float isolevel;
layout(binding = 0)buffer sf {
	float field[];
} scalar;

out int celltype;

void main(void) {
	int posIn1D = gl_VertexID;
	ivec3 posIn3D = ivec3(posIn1D % cellsPerAxis, (posIn1D / cellsPerAxis) % cellsPerAxis, posIn1D / (cellsPerAxis * cellsPerAxis));
	const ivec3 cornerOffsets[] = {
		ivec3(0, 0, 0),
		ivec3(1, 0, 0),
		ivec3(1, 0, 1),
		ivec3(0, 0, 1),
		ivec3(0, 1, 0),
		ivec3(1, 1, 0),
		ivec3(1, 1, 1),
		ivec3(0, 1, 1)
	};
	int cellTypeIndex = 0;
	for(int i = 0; i < 8; i++) {
		ivec3 offsetPos = posIn3D + cornerOffsets[i];
		float scalarValue2 = scalar.field[(offsetPos.z * (cellsPerAxis + 1) * (cellsPerAxis + 1)) + (offsetPos.y * (cellsPerAxis + 1)) + offsetPos.x];
		if(scalarValue2 < isolevel) {
			cellTypeIndex |= 1<<i;
		}
	}
	celltype = cellTypeIndex;
}