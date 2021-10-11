#include"Input.h"

Input::Input() {
	//�L�[���͗p�z��̒��g��0�ɂ���
	ZeroMemory(inputKey, sizeof(inputKey));
}

Input::~Input() {

}

void Input::update() {
	//WASD�̓��̓`�F�b�N
	inputKeyCheck(KEY_W);
	inputKeyCheck(KEY_A);
	inputKeyCheck(KEY_S);
	inputKeyCheck(KEY_D);

	//SHIFT(��)�̓��̓`�F�b�N
	inputKeyCheck(VK_LSHIFT);

	//�}�E�X�̓��̓`�F�b�N
	inputKeyCheck(VK_LBUTTON);
	inputKeyCheck(VK_RBUTTON);

	//�}�E�X�̈ړ��ʎ擾
	startPos = endPos;
	GetCursorPos(&endPos);
	dPos.x = endPos.x - startPos.x;
	dPos.y = endPos.y - startPos.y;
}

void Input::inputKeyCheck(WPARAM param) {
	if (GetAsyncKeyState(param) & checkKey) {
		inputKey[param] = 0x01;
	}
	else inputKey[param] = 0x00;
}

void Input::MouseLeftButtonDown(bool flag) {
	cameraRotateEnable = flag;
}