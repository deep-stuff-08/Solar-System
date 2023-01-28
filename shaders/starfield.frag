#version 460 core

in VS_OUT {
	vec4 color;
	flat int opLocation;
} fs_in;

layout(binding = 0)uniform sampler2D tex;

layout(location = 0)out vec4 color[6];

void main(void) {
	color[fs_in.opLocation] = fs_in.color * texture(tex, gl_PointCoord);
}