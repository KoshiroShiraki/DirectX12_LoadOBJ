/*
���͏����p�֐�
��{�I�ɂ�windows�v���V�[�W������̓��̓��b�Z�[�W

WASD -> �J�����ʒu�ړ�
�}�E�X���� -> �J���������]��
*/
#pragma once
#include<Windows.h>
#include<iostream>

//�Ȃ����L�[�{�[�h�̃A���t�@�x�b�g�L�[�ɑΉ������}�N����WindowsAPI�o�͒�`����Ă��Ȃ��̂ŁA�g�p���镪������`���Ă���
//�Q�� -> https://docs.microsoft.com/ja-jp/windows/win32/inputdev/virtual-key-codes
#define KEY_W 0x57
#define KEY_A 0x41
#define KEY_S 0x53
#define KEY_D 0x44

//���͏��
enum INPUT_STATE {
	INPUT_PRESSED, //�L�[��������Ă�����
	INPUT_RELEASED, //�L�[��������Ă�����
};

class Input {
public:
	Input();
	~Input();

private:
	char inputKey[256]; //�L�[��������Ă���Ԃ�inputKey[���̓L�[] = 0x01,�L�[��������Ă���Ԃ�inputKey[���̓L�[] = 0x00
	bool inputEnable; //���͎�t�t���O

public:
	void keyPressed(WPARAM param); //�L�[�������ꂽ
	void keyRelesed(WPARAM param); //�L�[�������ꂽ
	void enableInput(bool flag);  //�L�[���͂��󂯕t���邩
};


