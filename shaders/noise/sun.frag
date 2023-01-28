#version 460 core

in VS_OUT {
	vec3 N;
	vec3 L;
	vec3 texCoord;
} fs_in;

layout(binding = 0)uniform sampler3D samp;

uniform sampler3D Noise;
uniform vec3 Color1 = vec3(0.8, 0.6, 0.0);
uniform vec3 Color2 = vec3(0.6, 0.2, 0.0);
uniform float NoiseScale = 0.4;

layout(location = 0)out vec4 color;
layout(location = 1)out int objectID;

void main(void) {
	vec4 noisevec = texture(samp, fs_in.texCoord * NoiseScale);
	float intensity = abs(noisevec[0] - 0.25) +
					abs(noisevec[1] - 0.125) +
					abs(noisevec[2] - 0.0625);
	intensity = clamp(intensity * 9.0, 0.0, 1.0);
	vec3 finalcolor = mix(Color1, Color2, intensity) * 1.3;
	color = vec4(finalcolor, 40.0);
	objectID = 0;
}