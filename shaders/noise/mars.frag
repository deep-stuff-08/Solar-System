#version 460 core

in VS_OUT {
	vec3 N;
	vec3 L;
	vec3 texCoord;
} fs_in;

layout(binding = 0)uniform sampler3D samp;

uniform sampler3D Noise;
uniform vec3 Color1 = vec3(0.7, 0.25, 0.0);
uniform vec3 Color2 = vec3(0.4, 0.15, 0.0);
uniform float NoiseScale = 0.7;

layout(location = 0)out vec4 color;
layout(location = 1)out int objectID;

void main(void) {
	vec4 noisevec = texture(Noise, fs_in.texCoord * NoiseScale);
	float y = abs(fs_in.texCoord.y) + noisevec.y;
	float intensity = abs(noisevec[0] - 0.25) +	abs(noisevec[1] - 0.125) + abs(noisevec[2] - 0.0625) + abs(noisevec[3] - 0.03125);
	float sineval = sin(fs_in.texCoord.y * 6.0 + intensity * 12.0) * cos(fs_in.texCoord.x * 6.0 + intensity * 12.0) * cos(fs_in.texCoord.z * 6.0 + intensity * 12.0) * 0.5 + 0.5;
	vec3 fincolor = mix(Color1, Color2, sineval) * 1.2;
	fincolor = mix(fincolor, vec3(0.8), y * y * y * 0.8);
	vec3 N = normalize(fs_in.N);
	vec3 L = normalize(fs_in.L);
	color = vec4((0.1 + max(dot(N, L), 0.0)) * fincolor, 40.0);
	objectID = 4;
}