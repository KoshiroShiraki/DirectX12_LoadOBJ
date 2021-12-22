#pragma once
#include"Light.h"

Light::Light() {

}

Light::~Light() {

}

void Light::InitLight(XMFLOAT3 pos, XMFLOAT3 target, XMFLOAT3 up, XMFLOAT3 color) {
	m_pos = pos;
	m_target = target;
	m_up = up;
	m_LightViewMatrix = XMMatrixLookAtLH(XMLoadFloat3(&m_pos), XMLoadFloat3(&m_target), XMLoadFloat3(&m_up));

	m_color = color;
}