#pragma once
#include<DirectXMath.h>

using namespace DirectX;

class Light {
public :
	Light();
	~Light();

public :
	XMFLOAT3 m_pos;
	XMFLOAT3 m_target;
	XMFLOAT3 m_up;
	XMMATRIX m_LightViewMatrix;
	
	XMFLOAT3 m_color;


public: 
	void InitLight(XMFLOAT3 pos, XMFLOAT3 target, XMFLOAT3 up, XMFLOAT3 color);
	void update();
};