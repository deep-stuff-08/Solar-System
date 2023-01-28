#version 460 core

#define EPSILON 0.000001

layout(location = 2)uniform mat4 mMat;
layout(location = 1)uniform mat4 vMat;
layout(location = 0)uniform mat4 pMat;
layout(location = 3)uniform int cellsPerAxis;
layout(location = 4)uniform int numVertices;
layout(location = 5)uniform float isolevel;
layout(location = 6)uniform vec3 lightPosition = vec3(0.0, 0.0, 10.0);
layout(location = 7)uniform vec4 sphere[9];

layout(std430, binding = 0)buffer cti {
	int index[];
} celltype;
layout(std430, binding = 1)buffer sf {
	float field[];
} scalar;
layout(std430, binding = 2)buffer tt {
	int table[];
} tri;

out vec3 texCoord;

void main(void) {
	int vertexNo = gl_VertexID % numVertices;
	int posIn1D = gl_VertexID / numVertices;
	ivec3 posIn3D = ivec3(posIn1D % cellsPerAxis, (posIn1D / cellsPerAxis) % cellsPerAxis, posIn1D / (cellsPerAxis * cellsPerAxis));
	int celltype = celltype.index[posIn1D];
	int edgeNo = tri.table[celltype * numVertices + vertexNo];
	if(edgeNo != -1) {
		vec3 cellOrigin = vec3(-20.0) + vec3(posIn3D) * (40.0 / cellsPerAxis);
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
		const int edgestartkey[12] = int[] ( 0,1,2,3,4,5,6,7,0,1,2,3 );
		const int edgeendkey[12] = int[] ( 1,2,3,0,5,6,7,4,4,5,6,7 );

		ivec3 startCorner = posIn3D + cornerOffsets[edgestartkey[edgeNo]];
		ivec3 endCorner = posIn3D + cornerOffsets[edgeendkey[edgeNo]];
		float startCornerField = scalar.field[(cellsPerAxis + 1) * (cellsPerAxis + 1) * startCorner.z + (cellsPerAxis + 1) * startCorner.y + startCorner.x];
		float endCornerField = scalar.field[(cellsPerAxis + 1) * (cellsPerAxis + 1) * endCorner.z + (cellsPerAxis + 1) * endCorner.y + endCorner.x];
		float fieldDelta = abs(endCornerField - startCornerField);
		float middleRatio;
		if(fieldDelta > EPSILON) {
			middleRatio = abs(endCornerField - isolevel) / fieldDelta;
		} else {
			middleRatio = 0.5;
		}

		vec3 startCornerCoord = vec3(-20.0) + vec3(startCorner) * (40.0 / cellsPerAxis);
		vec3 endCornerCoord = vec3(-20.0) + vec3(endCorner) * (40.0 / cellsPerAxis);
		vec4 edgeMiddle = vec4(mix(endCornerCoord, startCornerCoord, middleRatio), 1.0);
		texCoord = (mMat * edgeMiddle).xyz;
		gl_Position = pMat * vMat * edgeMiddle;
	} else {
		gl_Position = vec4(0.0);
	}
}