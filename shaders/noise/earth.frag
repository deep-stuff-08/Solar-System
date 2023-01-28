#version 460 core

in VS_OUT {
	vec3 N;
	vec3 L;
	vec3 texCoord;
} fs_in;

layout(binding = 0)uniform sampler3D samp;

uniform sampler3D Noise;
uniform vec3 Color11 = vec3(0.0, 0.6, 0.0);
uniform vec3 Color12 = vec3(0.3, 0.4, 0.2);
uniform vec3 Color21 = vec3(0.0, 0.4, 0.6);
uniform vec3 Color22 = vec3(0.0, 0.0, 0.4);
uniform float NoiseScale = 0.5;

layout(location = 0)out vec4 color;
layout(location = 1)out int objectID;

void main(void) {
	vec4 noisevec = texture(samp, fs_in.texCoord * NoiseScale);
	float intensity = (noisevec[0] - 0.25) +
					(noisevec[1] - 0.125) +
					(noisevec[2] - 0.0625) +
					(noisevec[3] - 0.03125);
	vec3 finalcolor;
	if(intensity > 0.0) {
		finalcolor = mix(Color11, Color12, clamp(abs(intensity * 7.0), 0.0, 1.0));
	} else {
		finalcolor = mix(Color21, Color22, clamp(abs(intensity * 12.0), 0.0, 1.0));
	}
	vec3 N = normalize(fs_in.N);
	vec3 L = normalize(fs_in.L);
	color = vec4((0.1 + max(dot(N, L), 0.0)) * finalcolor, 40.0);
	objectID = 3;
}