#pragma once
#include<GL/gl.h>
#include<stdlib.h>
#include<math.h>

#define vertex_stride 8 * 4
#define position_offset (0)
#define normal_offset (3 * 4)
#define texcoord_offset (6 * 4)

class sphere {
private:
	GLfloat* vertices;
	GLuint* elements;
	GLuint numOfTriangles;
	size_t sizeOfVertices;
	size_t sizeOfElements;
public:
	sphere(GLushort stacks, GLushort slices, GLfloat radius, bool isTFlipped);
	GLfloat* getVertices();
	GLuint* getElements();
	GLuint getNumOfTriangle();
	size_t getSizeOfVertices();
	size_t getSizeOfElements();
};