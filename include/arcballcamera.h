#ifndef __arcball_camera__
#define __arcball_camera__

#include"vmathmod.h"

class ArcballCamera {
private:
	vmath::mat4 lastRotation;
	vmath::mat4 currentRotation;
	vmath::vec3 startVector;
	vmath::vec3 endVector;
	vmath::mat4 homeRotation;
	bool isDraggingPermitted;
public:
	inline ArcballCamera() {
		this->lastRotation = vmath::mat4::identity();
		this->currentRotation = vmath::mat4::identity();
	}
	inline vmath::mat4 getRotationMatrix() {
		return this->currentRotation;
	}
	void click(vmath::ivec2);
	void drag(vmath::ivec2);
	void rotateOnAxis(float rad, vmath::vec3 axis);
	void saveCurrentMatrix();
	inline void saveToHomeMatrix() {
		homeRotation = currentRotation;
	}
	inline void restoreFromHomeMatrix() {
		lastRotation = vmath::mat4::identity();
		currentRotation = homeRotation;
	}
	inline void disableDrag() {
		isDraggingPermitted = false;
	}
	inline void enableDrag() {
		isDraggingPermitted = true;
	}
};

#endif