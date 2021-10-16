/*
���͏����p�֐�
��{�I�ɂ�windows�v���V�[�W������̓��̓��b�Z�[�W

WASD -> �J�����ʒu�ړ�
�}�E�X���� -> �J���������]��
*/
#pragma once
#include<Windows.h>
#include<iostream>
#include<DirectXMath.h>

using namespace DirectX;

//�Ȃ����L�[�{�[�h�̃A���t�@�x�b�g�L�[�ɑΉ������}�N����WindowsAPI�o�͒�`����Ă��Ȃ��̂ŁA�g�p���镪������`���Ă���
//�Q�� -> https://docs.microsoft.com/ja-jp/windows/win32/inputdev/virtual-key-codes
#define KEY_W 0x57
#define KEY_A 0x41
#define KEY_S 0x53
#define KEY_D 0x44
#define KEY_E 0x45

//���͏��
enum INPUT_STATE {
	INPUT_PRESSED, //�L�[��������Ă�����
	INPUT_RELEASED, //�L�[��������Ă�����
};

class Input {
public:
	Input();
	~Input();

public:
	char inputKey[256]; //�L�[��������Ă���Ԃ�inputKey[���̓L�[] = 0x01,�L�[��������Ă���Ԃ�inputKey[���̓L�[] = 0x00
	bool inputEnable; //���͎�t�t���O
	bool cameraRotateEnable; //�E�N���b�N����Ă���Ԃ́A�J��������������󂯕t����

	POINT startPos; //1�t���[���O�̃}�E�X�J�[�\�����W(���_)
	POINT endPos; //���݃t���[���̃}�E�X�J�[�\�����W(�I�_)
	POINT dPos; //���W�̕ω���
	
private:
	const int checkKey = 0x8000; //32�r�b�g�̍ŏ�ʃr�b�g��1 = ������Ă���Ȃ̂ł��̔���p�Ɏg��

public:
	void update();
	void inputKeyCheck(WPARAM param);
	void inputMouseCheck(WPARAM param);

	void MouseLeftButtonDown(bool flag); //�}�E�X���E�N���b�N����Ă���
};


