#pragma once
#include"Camera.h"

Camera::Camera() {
}

Camera::~Camera() {

}

void Camera::InitCamera(XMFLOAT3 apos, XMFLOAT3 atarget, XMFLOAT3 aup) {
	pos = apos;
	target = atarget;
	up = aup;

	verticalDir = 0.0f;

	moveSpeed = 1.0f;
	cameraSensitivity = 0.005f;

	/*-----Create ViewMatrix-----*/
	viewMatrix = viewMatrixRotateY = XMMatrixLookAtLH(DirectX::XMLoadFloat3(&pos), DirectX::XMLoadFloat3(&target), DirectX::XMLoadFloat3(&up));
}

/*
this Application, moving input is W/A/S/D and Shift or not.
move Without Shift, camera move horizontal direction.
move with Shift, camera move Vertical direction.
so, not to move to camera's Direction, Matrix is separated ViewMatrix Rotated only y axis(viewMatrixRotateY) from ViewMatrix rotated x/y axis(viewMatrix)
*/
void Camera::update(XMFLOAT3 move, XMFLOAT3 rot, bool rotateFlag) {
	//normalize move Vector
	float len = sqrtf(pow(move.x, 2) + pow(move.y, 2) + pow(move.z, 2));
	if (len == 0) len = 1; //

	rot.x *= cameraSensitivity;
	rot.y *= cameraSensitivity;
	rot.z *= cameraSensitivity;

	XMMATRIX moveMat = XMMatrixTranslation(move.x / len, move.y / len, move.z / len);
	XMMATRIX rotateYMat = XMMatrixIdentity();
	XMMATRIX rotateXMat = XMMatrixIdentity();

	if (rotateFlag) { //Camera roatation is only excuted when Mouse's right Button Clicked
		rotateYMat = XMMatrixRotationY(-rot.y);
		verticalDir += rot.z;
	}
	rotateXMat = XMMatrixRotationX(-verticalDir);
	viewMatrixRotateY *= rotateYMat * moveMat;
	viewMatrix = viewMatrixRotateY * rotateXMat;

}