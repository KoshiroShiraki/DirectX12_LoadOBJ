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
	m_viewMatrix = XMMatrixLookAtLH(XMLoadFloat3(&m_pos), XMLoadFloat3(&m_target), XMLoadFloat3(&m_up));
	m_baseViewMatrix = m_viewMatrix;

	m_color = color;
}

void Light::update(float* data) {
	//ライト行列は毎回作り直す
	m_pos = XMFLOAT3(-data[23], -data[24], -data[25]); 
	m_viewMatrix = m_viewMatrix = XMMatrixLookAtLH(XMLoadFloat3(&m_pos), XMLoadFloat3(&m_target), XMLoadFloat3(&m_up));
}

void Light::updateColor(float* data) {
	//渡されるデータの20~22に目的のデータがある
	m_color = XMFLOAT3(data[20], data[21], data[22]);
}