#pragma once
#include "Module.h"
class ModuleEditorCamera : public Module
{
	struct Params
	{
		float polar;
		float azimuthal;
		Vector3 panning;
	};

public:
	void setFOV(float _fov);
	void setAspectRatio(float _aspect);
	void setPlaneDistance(float _nearPlane, float _farPlane);
	const Vector3& Position() { return position; };
	const Quaternion& Orientation() { return rotation; }
	void LookAt(float, float, float);
	const Matrix GetProjectionMatrix(float aspect);
	const Matrix& GetViewMatrix() { return view; }
	virtual bool init() override;
	virtual void update() override;

private:

	Vector3 position;
	Quaternion rotation;
	Matrix view;

	float nearPlane = 0.1f;
	float farPlane = 20000.0f;
	float fov = XM_PIDIV4;
	float aspect;

	int DragPosX;
	int DragPosY;

	Params params = { 0.0f, 0.0f, {0.0f, 1.0f, 10.0f} };

	

};

