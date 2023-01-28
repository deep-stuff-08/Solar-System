#ifndef __HELPERFUNCS__
#define __HELPERFUNCS__

#include"render.h"
#include<stdio.h>
#include<math.h>

static unsigned int seed = 0x13371339;

float random_float() {
	float res;
	unsigned int tmp;
	seed *= 16807;
	tmp = seed ^ (seed >> 4) ^ (seed << 15);
	*((unsigned int *) &res) = (tmp >> 9) | 0x3F800000;
	return (res - 1.0f);
}

int mousePicker(int cx, int cy, int resolution, int fbo) {
	int x = cx * ((float)resolution / winSize.w);
	int y = ((int)winSize.h - cy) * ((float)resolution / winSize.h);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glReadBuffer(GL_COLOR_ATTACHMENT1);
	int data;
	glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &data);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return data;
}

void calculateWeightsForGaussianKernel(float sigma, float *weights, int len) {
	int i = 0;
	float sumWeights = 0.0f;
	for(int x = (len - 1) / 2; x >= 0; x--, i++) {
		weights[i] = exp(-((x * x) / (2 * sigma * sigma))) / sqrt(2 * M_PI * sigma * sigma);
		sumWeights += weights[i];
	}
	for(int j = i - 2; j >= 0; j--, i++) {
		weights[i] = weights[j];
		sumWeights += weights[i];
	}
	for(int j = 0; j < len; j++) {
		weights[j] = weights[j] / sumWeights;
	}
}

#endif