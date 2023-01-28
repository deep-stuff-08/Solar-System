#include"include/arcballcamera.h"
#include"include/render.h"


vmath::vec3 vectorFromPoint(vmath::ivec2 point) {
	const float radius = 1.0f;
	vmath::vec2 viewPoints = vmath::vec2(((float)point[0]/winSize.w) * 2.0f - 1.0f, -(((float)point[1]/winSize.h) * 2.0f - 1.0f));
	float len = length(viewPoints) * length(viewPoints);
	if(len > radius) {
		return vmath::vec3(normalize(viewPoints), 0.0f);
	} else {
		return vmath::vec3(viewPoints, sqrt(radius * radius - len));
	}
}

void ArcballCamera::click(vmath::ivec2 startPoint) {
	saveCurrentMatrix();
	startVector = vectorFromPoint(startPoint);
}

void ArcballCamera::saveCurrentMatrix() {
	this->lastRotation = this->currentRotation;
}

void ArcballCamera::drag(vmath::ivec2 endPoint) {
	if(isDraggingPermitted) {
		endVector = vectorFromPoint(endPoint);

		vmath::vec3 perpendicular = vmath::cross(startVector, endVector);
		float theta = acos(vmath::dot(startVector, endVector));
		rotateOnAxis(theta, perpendicular);
	}
}

void ArcballCamera::rotateOnAxis(float rad, vmath::vec3 axis) {
	if(vmath::length(axis) > 1.0e-7) {
		axis = vmath::normalize(axis);
		rad = rad / 2.0f;
		vmath::quaternionToMatrix(vmath::quaternion(cos(rad), axis * (float)sin(rad)), this->currentRotation);
	} else {
		this->currentRotation = vmath::mat4::identity();
	}
	this->currentRotation *= this->lastRotation;
}