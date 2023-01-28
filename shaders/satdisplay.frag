#version 450 core

in vec2 texCoords;

layout(location = 0) uniform float focalDist;
layout(location = 1) uniform float focalDepth;
layout(binding = 0) uniform sampler2D inputImage;

layout(location = 0) out vec4 color;

void main(void) {
	vec2 s = 1.0 / textureSize(inputImage, 0);
	vec2 C = gl_FragCoord.xy;
	vec4 v = texelFetch(inputImage, ivec2(gl_FragCoord.xy), 0).rgba;

	float m;

	if(v.w == 0.0) {
		m = 0.5;
	} else {
		m = abs(v.w - focalDist);
		m = 0.5 + smoothstep(0.0, focalDepth, m) * 7.5;
	}

	vec2 P0 = vec2(C * 1.0) + vec2(-m, -m);
    vec2 P1 = vec2(C * 1.0) + vec2(-m, m);
    vec2 P2 = vec2(C * 1.0) + vec2(m, -m);
    vec2 P3 = vec2(C * 1.0) + vec2(m, m);

    P0 *= s;
    P1 *= s;
    P2 *= s;
    P3 *= s;

	vec3 a = textureLod(inputImage, P0, 0).rgb;
	vec3 b = textureLod(inputImage, P1, 0).rgb;
	vec3 c = textureLod(inputImage, P2, 0).rgb;
	vec3 d = textureLod(inputImage, P3, 0).rgb;

	vec3 f = a - b - c + d;

	m *= 2.0;
	f /= float(m * m);
	color = vec4(f, 1.0);
	// color = texture(inputImage, texCoords);
}