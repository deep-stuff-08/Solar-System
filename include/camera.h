#pragma once

#include"vmathmod.h"

class Camera {
private:
	vmath::vec3 x_vec;
	vmath::vec3 y_vec;
	vmath::vec3 z_vec;
	float pitch;
	float yaw;
	float roll;
	vmath::vec3 position;
public:
	//Camera();
	Camera(vmath::vec3 position, vmath::vec3 front, vmath::vec3 up);
	void move(vmath::vec3 m);
	void move(float mx, float my, float mz);
	void turn(vmath::vec3 rads);
	void turn(float radPitch, float radYaw, float radRoll);
	vmath::mat4 getCurrentCamera();
};
