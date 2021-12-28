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

	viewMatrix = viewMatrixRotateY = XMMatrixLookAtLH(DirectX::XMLoadFloat3(&pos), DirectX::XMLoadFloat3(&target), DirectX::XMLoadFloat3(&up));
}

void Camera::update(XMFLOAT3 move, XMFLOAT3 rot, bool rotateFlag) {
	float len = sqrtf(pow(move.x, 2) + pow(move.y, 2) + pow(move.z, 2));
	if (len == 0) len = 1;

	rot.x *= cameraSensitivity;
	rot.y *= cameraSensitivity;
	rot.z *= cameraSensitivity;

	XMMATRIX moveMat = XMMatrixTranslation(move.x / len, move.y / len, move.z / len);
	XMMATRIX rotateYMat = XMMatrixIdentity();
	XMMATRIX rotateXMat = XMMatrixIdentity();

	if (rotateFlag) {
		rotateYMat = XMMatrixRotationY(-rot.y);
		verticalDir += rot.z;
	}
	rotateXMat = XMMatrixRotationX(-verticalDir);
	viewMatrixRotateY *= rotateYMat * moveMat;
	viewMatrix = viewMatrixRotateY * rotateXMat;

	//ビュー行列からカメラの位置情報を取り出す
	m_curPos = XMLoadFloat4(new XMFLOAT4(viewMatrix.r[3].m128_f32[0], viewMatrix.r[3].m128_f32[1], viewMatrix.r[3].m128_f32[2], 0));
	m_curPos = XMVector4Transform(m_curPos, viewMatrix);
	
}