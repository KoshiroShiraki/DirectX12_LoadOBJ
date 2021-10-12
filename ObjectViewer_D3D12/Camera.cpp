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

	/*-----view行列の生成-----*/
	viewMatrix = viewMatrixRotateY = XMMatrixLookAtLH(DirectX::XMLoadFloat3(&pos), DirectX::XMLoadFloat3(&target), DirectX::XMLoadFloat3(&up));
}

/*
addPosは移動量で、zが正面への移動量、xが横の移動量、yが縦の移動量
addPosを現在のカメラ方向へ回転させて、カメラの向いているほうが前方向になるようにする
*/
void Camera::update(XMFLOAT3 addPos, XMFLOAT3 addRot, bool rotateFlag) {
	//移動量の正規化
	float len = sqrtf(pow(addPos.x, 2) + pow(addPos.y, 2) + pow(addPos.z, 2));
	if (len == 0) len = 1; //移動量curPosの大きさが0の場合

	//SIMD演算はまた今度...
	addRot.x *= cameraSensitivity;
	addRot.y *= cameraSensitivity;
	addRot.z *= cameraSensitivity;

	XMMATRIX moveMat = XMMatrixTranslation(addPos.x / len, addPos.y / len, addPos.z / len);
	XMMATRIX rotateYMat = XMMatrixIdentity();
	XMMATRIX rotateXMat = XMMatrixIdentity();

	if (rotateFlag) { //回転計はマウスクリックがあるときだけ
		rotateYMat = XMMatrixRotationY(-addRot.y); //y軸の回転をして
		verticalDir += addRot.z; //垂直方向の回転量を更新する
	}
	rotateXMat = XMMatrixRotationX(-verticalDir);
	viewMatrixRotateY *= rotateYMat * moveMat;
	viewMatrix = viewMatrixRotateY * rotateXMat;

}