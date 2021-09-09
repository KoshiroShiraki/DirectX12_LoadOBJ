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
	
	/*-----viewçsóÒÇÃê∂ê¨-----*/
	viewMatrix = XMMatrixLookAtLH(DirectX::XMLoadFloat3(&pos), DirectX::XMLoadFloat3(&target), DirectX::XMLoadFloat3(&up));
}