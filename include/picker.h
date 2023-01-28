#ifndef __PICKER__
#define __PICKER__

#include"vmathmod.h"
#include"render.h"

vmath::vec3 mousePointAsRay(vmath::ivec2 xy, vmath::mat4 projection, vmath::mat4 view) {
	vmath::vec2 nds = vmath::vec2((2.0f * xy[0]) / winSize.w - 1.0f, 1.0f - ((2.0f * xy[1]) / winSize.h));
	vmath::vec4 clip = vmath::vec4(nds, -1.0f, 1.0f);
	vmath::vec4 eye = clip * vmath::inverse(projection);
	eye[2] = -1.0f; eye[3] = 0.0f;
	vmath::vec4 world = eye * vmath::inverse(view);
	vmath::vec3 worldVec3 = vmath::vec3(world[0], world[1], world[2]);
	worldVec3 = vmath::normalize(worldVec3);
	// printf("%f %f %f\n", worldVec3[0], worldVec3[1], worldVec3[2]);
	return worldVec3;
}


bool checkIntersection(vmath::vec3 rayOrigin, vmath::vec3 rayDirection, vmath::vec3 sphereCenter, GLfloat sphereRadius, GLfloat* distance) {
	vmath::vec3 distToSphere = rayOrigin - sphereCenter;
	float b = vmath::dot(rayDirection, distToSphere);
	float c = vmath::dot(distToSphere, distToSphere) - sphereRadius * sphereRadius;
	float b_squared_minus_c = b * b - c;
	if (b_squared_minus_c < 0.0f) { 
		// printf("%f\n", b_squared_minus_c);
		return false;
	} else {
		float t_a = -b + sqrt(b_squared_minus_c);
		float t_b = -b - sqrt(b_squared_minus_c);
		*distance = t_b;
		if (t_a <= 0.0) {
			if ( t_b < 0.0 ) {
				return false;
			}
		} else if (t_b < 0.0) {
			*distance = t_a;
		}
		return true;
	}
	return false;
}

#endif