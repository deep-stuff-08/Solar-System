#version 460 core

layout(binding = 0, rgba32f) uniform image2D inputImg;
layout(binding = 1, rgba32f) uniform image2D outputImg;
layout(location = 0) uniform ivec2 offset;

void main(void) {
	ivec2 id = ivec2(gl_FragCoord.x, gl_FragCoord.y);
	vec4 i = imageLoad(inputImg, id).rgba;
	vec3 sum = i.rgb + imageLoad(inputImg, id - offset).rgb;
	imageStore(outputImg, id, vec4(sum, i.a));
}