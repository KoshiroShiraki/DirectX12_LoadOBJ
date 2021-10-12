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

	/*-----view�s��̐���-----*/
	viewMatrix = viewMatrixRotateY = XMMatrixLookAtLH(DirectX::XMLoadFloat3(&pos), DirectX::XMLoadFloat3(&target), DirectX::XMLoadFloat3(&up));
}

/*
addPos�͈ړ��ʂŁAz�����ʂւ̈ړ��ʁAx�����̈ړ��ʁAy���c�̈ړ���
addPos�����݂̃J���������։�]�����āA�J�����̌����Ă���ق����O�����ɂȂ�悤�ɂ���
*/
void Camera::update(XMFLOAT3 addPos, XMFLOAT3 addRot, bool rotateFlag) {
	//�ړ��ʂ̐��K��
	float len = sqrtf(pow(addPos.x, 2) + pow(addPos.y, 2) + pow(addPos.z, 2));
	if (len == 0) len = 1; //�ړ���curPos�̑傫����0�̏ꍇ

	//SIMD���Z�͂܂����x...
	addRot.x *= cameraSensitivity;
	addRot.y *= cameraSensitivity;
	addRot.z *= cameraSensitivity;

	XMMATRIX moveMat = XMMatrixTranslation(addPos.x / len, addPos.y / len, addPos.z / len);
	XMMATRIX rotateYMat = XMMatrixIdentity();
	XMMATRIX rotateXMat = XMMatrixIdentity();

	if (rotateFlag) { //��]�v�̓}�E�X�N���b�N������Ƃ�����
		rotateYMat = XMMatrixRotationY(-addRot.y); //y���̉�]������
		verticalDir += addRot.z; //���������̉�]�ʂ��X�V����
	}
	rotateXMat = XMMatrixRotationX(-verticalDir);
	viewMatrixRotateY *= rotateYMat * moveMat;
	viewMatrix = viewMatrixRotateY * rotateXMat;

}