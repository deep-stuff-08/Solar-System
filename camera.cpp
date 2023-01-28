#include"include/camera.h"
#include<stdio.h>

// Camera::Camera() {
	// this->x_vec = vmath::vec3(1.0f, 0.0f, 0.0f);
	// this->y_vec = vmath::vec3(0.0f, 1.0f, 0.0f);
	// this->z_vec = vmath::vec3(0.0f, 0.0f, 1.0f);
	// this->position = vmath::vec3(0.0f);
	// this->pitch = 0.0f;
	// this->yaw = -90.0f;
	// this->roll = 0.0f;
// }

Camera::Camera(vmath::vec3 position, vmath::vec3 front, vmath::vec3 up) {
	this->position = position;
	
	this->pitch = 0.0f;
	this->yaw = -90.0f;
	this->roll = 0.0f;

	this->x_vec = vmath::vec3(1.0f, 0.0f, 0.0f);
	this->y_vec = vmath::vec3(0.0f, 1.0f, 0.0f);
	this->z_vec = vmath::vec3(0.0f, 0.0f, -1.0f);
}

void Camera::move(vmath::vec3 m) {
	move(m[0], m[1], m[2]);
}

void Camera::move(float mx, float my, float mz) {
	vmath::vec3 right;
	if(mx != 0) {
		this->position += mx * this->x_vec;
	}
	if(my != 0) {
		this->position += my * this->y_vec;	
	}
	if(mz != 0) {
		this->position += mz * this->z_vec;	
	}
}

void Camera::turn(vmath::vec3 deg) {
	turn(deg[0], deg[1], deg[2]);
}

void Camera::turn(float degPitch, float degYaw, float degRoll) {
	float radPitch = vmath::radians(degPitch);
	float radYaw = vmath::radians(degYaw);
	float radRoll = vmath::radians(degRoll);

	this->pitch += degPitch;

	vmath::mat3 rollMat = vmath::mat3(
		vmath::vec3(cos(radRoll), sin(radRoll), 0),
		vmath::vec3(-sin(radRoll), cos(radRoll), 0),
		vmath::vec3(0, 0, 1)
	);

	vmath::mat3 yawMat = vmath::mat3(
		vmath::vec3(cos(radYaw), 0, -sin(radYaw)),
		vmath::vec3(0, 1, 0),
		vmath::vec3(sin(radYaw), 0, cos(radYaw))
	);
	
	vmath::mat3 pitchMat = vmath::mat3(
		vmath::vec3(1, 0, 0),
		vmath::vec3(0, cos(radPitch), sin(radPitch)),
		vmath::vec3(0, -sin(radPitch), cos(radPitch))
	);

	this->x_vec = (rollMat * this->x_vec)[0];
	this->y_vec = (rollMat * this->y_vec)[0];
	this->z_vec = (rollMat * this->z_vec)[0];

	this->x_vec = (yawMat * this->x_vec)[0];
	this->y_vec = (yawMat * this->y_vec)[0];
	this->z_vec = (yawMat * this->z_vec)[0];

	this->x_vec = (pitchMat * this->x_vec)[0];
	this->y_vec = (pitchMat * this->y_vec)[0];
	this->z_vec = (pitchMat * this->z_vec)[0];
}

vmath::mat4 Camera::getCurrentCamera() {
	return vmath::mat4(
		vmath::vec4(this->x_vec[0], this->y_vec[0], -this->z_vec[0], 0.0f),
		vmath::vec4(this->x_vec[1], this->y_vec[1], -this->z_vec[1], 0.0f),
		vmath::vec4(this->x_vec[2], this->y_vec[2], -this->z_vec[2], 0.0f),
		vmath::vec4(-this->position[0], -this->position[1], -this->position[2], 1.0f)
	);
}