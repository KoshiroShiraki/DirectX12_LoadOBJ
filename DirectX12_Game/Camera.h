/*
☆Camera管理クラス
主にView行列の管理
*/

#pragma once
#include<DirectXMath.h>

using namespace DirectX;

class Camera {
public:
	/*-----�����o�ϐ�-----*/
	XMMATRIX viewMatrix;
	XMFLOAT3 pos;
	XMFLOAT3 target;
	XMFLOAT3 up;

	/*-----�R���X�g���N�^/�f�X�g���N�^-----*/
	Camera();
	~Camera();

	/*-----�����o�֐�-----*/
	void InitCamera(XMFLOAT3 apos, XMFLOAT3 atarget, XMFLOAT3 aup);
};