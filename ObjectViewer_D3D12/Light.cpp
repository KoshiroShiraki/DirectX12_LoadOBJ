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
	//ベクトルの正規化
	m_pos = XMFLOAT3(-data[23], -(data[24] + 0.01f), -data[25]);
	float length = sqrtf(powf(m_pos.x, 2) + powf(m_pos.y, 2) + powf(m_pos.z, 2));
	if (length == 0) length = 1;
	m_pos.x = (m_pos.x / length) * 500;
	m_pos.y = (m_pos.y / length) * 500;
	m_pos.z = (m_pos.z / length) * 500;

	m_viewMatrix = m_viewMatrix = XMMatrixLookAtLH(XMLoadFloat3(&m_pos), XMLoadFloat3(&m_target), XMLoadFloat3(&m_up));
}

void Light::updateColor(float* data) {
	//渡されるデータの20~22に目的のデータがある
	m_color = XMFLOAT3(data[20], data[21], data[22]);
}