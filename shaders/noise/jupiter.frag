#version 460 core

in VS_OUT {
	vec3 N;
	vec3 L;
	vec3 texCoord;
} fs_in;

layout(binding = 0)uniform sampler3D samp;

uniform sampler3D Noise;
uniform vec3 Color1 = vec3(0.52, 0.4, 0.2);
uniform vec3 Color2 = vec3(0.52, 0.5, 0.4);
uniform vec3 Color3 = vec3(0.77, 0.74, 0.7);
uniform vec3 Color4 = vec3(0.8, 0.8, 0.8);
uniform float NoiseScale = 0.7;

layout(location = 0)out vec4 color;
layout(location = 1)out int objectID;

void main(void) {
	vec4 noisevec = texture(Noise, fs_in.texCoord * NoiseScale);
	float intensity = abs(noisevec[0] - 0.25) +	abs(noisevec[1] - 0.125) + abs(noisevec[2] - 0.0625) + abs(noisevec[3] - 0.03125);
	float sineval = sin(fs_in.texCoord.y * 8.0 + intensity * 8.0) * 0.5 + 0.5;
	vec3 fincolor1 = mix(Color1, Color2, sineval);
	float coseval = cos(fs_in.texCoord.y * 8.0 + intensity * 8.0) * 0.5 + 0.5;
	vec3 fincolor2 = mix(Color3, Color4, coseval);
	float sincoseval = sin(fs_in.texCoord.y * 8.0 + intensity * 8.0) * cos(fs_in.texCoord.y * 8.0 + intensity * 8.0) * 0.5 + 0.5;
	vec3 fincolor = mix(fincolor1, fincolor2, sincoseval) * 1.2;
	vec3 N = normalize(fs_in.N);
	vec3 L = normalize(fs_in.L);
	color = vec4((0.1 + max(dot(N, L), 0.0)) * fincolor, 40.0);
	objectID = 5;
}