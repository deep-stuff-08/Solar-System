#pragma once

#include<iostream>
#include<fstream>
#include<string>

using namespace std;

#include<GL/glew.h>
#include<GL/gl.h>
#include<string.h>

#define len(x) (sizeof(x)/sizeof(x[0]))

enum gldlError {
	GLDL_SUCCESS,
	GLDL_FILE_NOT_FOUND_OR_ERROR,
	GLDL_NO_SHADERS,
	GLDL_COMPILER_ERROR,
	GLDL_LINKER_ERROR
};

struct ShaderInfo {
	GLenum type;
	string filename;
};

GLuint currentShader;
GLuint currentProgram;
gldlError currentErr;
string filename;
string currentProgramName;

GLint gldlCreateShaderProgram(ShaderInfo *shaders, const unsigned int length, string progName) {
	currentProgramName = progName;
	if(!shaders || !length) {
		currentErr = GLDL_NO_SHADERS;
		return -1;
	}
	GLuint program = glCreateProgram();
	for(int i = 0; i < length; i++) {
		ifstream file(shaders[i].filename, ifstream::in);
		if(!file.is_open()) {
			filename = shaders[i].filename;
			currentErr = GLDL_FILE_NOT_FOUND_OR_ERROR;
			return -1;
		}
		file.seekg(0, ios_base::end);
		int flen = file.tellg();
		file.seekg(0, ios_base::beg);
		GLchar* source = new GLchar[flen+1];
		file.read((char*)source, flen);
		file.close();
		GLuint shader = glCreateShader(shaders[i].type);
		glShaderSource(shader, 1, &source, &flen);
		delete source;
		glCompileShader(shader);
		GLint compiledStatus;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiledStatus);
		if(!compiledStatus) {
			currentShader = shader;
			filename = shaders[i].filename;
			currentErr = GLDL_COMPILER_ERROR;
			return -1;
		}
		glAttachShader(program, shader);
	}
	glLinkProgram(program);
	GLint linkedStatus;
	glGetProgramiv(program, GL_LINK_STATUS, &linkedStatus);
	if(!linkedStatus) {
		currentProgram = program;
		currentErr = GLDL_LINKER_ERROR;
		return -1;
	}
	currentErr = GLDL_SUCCESS;
	return program;
}

void gldlDeleteShaderProgram(GLuint program) {
	if(!program) {
		return;
	}
	glUseProgram(program);
	GLsizei shaderCount;
	glGetProgramiv(program, GL_ATTACHED_SHADERS, &shaderCount);
	GLuint* pShader = (GLuint*)malloc(sizeof(GLuint) * shaderCount);
	if(pShader) {
		glGetAttachedShaders(program, shaderCount, &shaderCount, pShader);
		for(GLsizei i = 0; i < shaderCount; i++) {
			glDetachShader(program, pShader[i]);
			glDeleteShader(pShader[i]);
		}
		free(pShader);
	}
	glDeleteProgram(program);
	glUseProgram(0);
}

void gldlPrintErrorMessage(void) {
	int len;
	char errormsg[1024];
	switch(currentErr) {
	case GLDL_SUCCESS:
		printf("%s: successfully compiled and linked\n", currentProgramName.c_str());
		break;
	case GLDL_FILE_NOT_FOUND_OR_ERROR:
		printf("%s: %s doesn't exists\n", currentProgramName.c_str(), filename.c_str());
		break;
	case GLDL_NO_SHADERS:
		printf("%s: struct empty or lenght zero\n", currentProgramName.c_str());
		break;
	case GLDL_COMPILER_ERROR:
		glGetShaderInfoLog(currentShader, 1000, &len, errormsg);
		printf("%s: %s returned by compiler while compiling '%s'\n", currentProgramName.c_str(), errormsg, filename.c_str());
		break;
	case GLDL_LINKER_ERROR:
		glGetProgramInfoLog(currentProgram, 1000, &len, errormsg);
		printf(errormsg, "%s: %s is returned by linker\n", currentProgramName.c_str(), errormsg);
		break;
	default:	
		printf("%s: unknown error\n", currentProgramName.c_str());
		break;
	}
}