#include"include/sphere.h"

sphere::sphere(GLushort stacks, GLushort slices, GLfloat radius, bool isTFlipped) {
	this->vertices = (GLfloat*)calloc(slices * stacks * 8, sizeof(GLfloat));
	this->sizeOfVertices = slices * stacks * 8 * sizeof(GLfloat);
	for(int i = 0; i < stacks; i++) {
		GLfloat phi = M_PI * ((GLfloat)i / (stacks - 1));
		for(int j = 0; j < slices; j++) {
			GLfloat theta = 2.0f * M_PI * ((GLfloat)j / (slices - 1));
			//Position
			vertices[(i * slices + j) * 8 + 0] = sin(phi) * sin(theta) * radius;
			vertices[(i * slices + j) * 8 + 1] = cos(phi) * radius;
			vertices[(i * slices + j) * 8 + 2] = sin(phi) * cos(theta) * radius;
			//Normal
			vertices[(i * slices + j) * 8 + 3] = sin(phi) * sin(theta);
			vertices[(i * slices + j) * 8 + 4] = cos(phi);
			vertices[(i * slices + j) * 8 + 5] = sin(phi) * cos(theta);
			//Texcoord
			vertices[(i * slices + j) * 8 + 6] = ((GLfloat)j / (slices - 1));
			if(isTFlipped) {
				vertices[(i * slices + j) * 8 + 7] = ((GLfloat)i / (stacks - 1));
			} else {
				vertices[(i * slices + j) * 8 + 7] = 1.0f - ((GLfloat)i / (stacks - 1));
			}
		}
	}

	this->elements = (GLuint*)calloc((slices - 1) * (stacks - 1) * 6, sizeof(GLuint));
	this->sizeOfElements = (slices - 1) * (stacks - 1) * 6 * sizeof(GLuint);
	this->numOfTriangles = (slices - 1) * (stacks - 1) * 6;
	for(int i = 0; i < stacks - 1; i++) {
		for(int j = 0; j < slices - 1; j++) {
			elements[(i * (slices - 1) + j) * 6 + 0] = i * slices + j;
			elements[(i * (slices - 1) + j) * 6 + 1] = (i + 1) * slices + j;
			elements[(i * (slices - 1) + j) * 6 + 2] = (i + 1) * slices + j + 1;

			elements[(i * (slices - 1) + j) * 6 + 3] = (i + 1) * slices + j + 1;
			elements[(i * (slices - 1) + j) * 6 + 4] = i * slices + j + 1;
			elements[(i * (slices - 1) + j) * 6 + 5] = i * slices + j;
		}
	}
}

GLfloat* sphere::getVertices() {
	return this->vertices;
}

GLuint* sphere::getElements() {
	return this->elements;
}

GLuint sphere::getNumOfTriangle() {
	return this->numOfTriangles;
}

size_t sphere::getSizeOfVertices() {
	return this->sizeOfVertices;
}

size_t sphere::getSizeOfElements() {
	return this->sizeOfElements;
}
