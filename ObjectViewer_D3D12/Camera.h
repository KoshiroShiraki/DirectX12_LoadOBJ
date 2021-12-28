#pragma once
#include<DirectXMath.h>
#include<iostream>

using namespace DirectX;

class Camera {
public:
	Camera();
	~Camera();

	XMMATRIX viewMatrix;	//View Matrix
	XMMATRIX viewMatrixRotateY;	//View Matrix which is rotated y axis
	XMFLOAT3 pos;	//cuurent Camera pos
	XMFLOAT3 target; //Camera Look at
	XMFLOAT3 up; //Camera up vector

	XMVECTOR m_curPos;

	float verticalDir;
	float moveSpeed;
	float cameraSensitivity;

	void InitCamera(XMFLOAT3 apos, XMFLOAT3 atarget, XMFLOAT3 aup); //apos = Camera Position, atarget = Camera Look at, aup = Camera Up Vector
	void update(XMFLOAT3 move, XMFLOAT3 rot, bool rotateFlag); //move = moveDirection, rot = rotate value, rotateFlag = Flag used for Mouse Light Button Clicked
};