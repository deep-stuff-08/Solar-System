#ifndef __RENDER__
#define __RENDER__

void initGL(void);
void renderGL(void);
void keyboardGL(int);
void mousemoveGL(int, int, bool);
void uninitGL(void);

struct winParam {
	float w;
	float h;
};

extern struct winParam winSize;

extern bool doDisplayFPS;

#endif