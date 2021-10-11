/*
☆Camera管理クラス
主にView行列の管理
*/

#pragma once
#include<DirectXMath.h>
#include<iostream>

using namespace DirectX;

class Camera {
public:
	/*-----�����o�ϐ�-----*/
	XMMATRIX viewMatrix; //現在のビュー行列
	XMMATRIX viewMatrixRotateY; //x軸回転していないビュー行列()
	XMFLOAT3 pos; //現在位置
	XMFLOAT3 target;
	XMFLOAT3 up;

	float verticalDir; //垂直方向の向き
	/*-----�R���X�g���N�^/�f�X�g���N�^-----*/
	Camera();
	~Camera();

	/*-----�����o�֐�-----*/
	void InitCamera(XMFLOAT3 apos, XMFLOAT3 atarget, XMFLOAT3 aup);
	void update(XMFLOAT3 curPos, XMFLOAT3 curRot, bool rotateFlag);
};