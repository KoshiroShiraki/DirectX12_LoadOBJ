#include"Input.h"

Input::Input() {
	//キー入力用配列の中身を0にする
	ZeroMemory(inputKey, sizeof(inputKey));
}

Input::~Input() {

}

void Input::update() {
	//WASDの入力チェック
	inputKeyCheck(KEY_W);
	inputKeyCheck(KEY_A);
	inputKeyCheck(KEY_S);
	inputKeyCheck(KEY_D);

	//SHIFT(左)の入力チェック
	inputKeyCheck(VK_LSHIFT);

	//マウスの入力チェック
	inputKeyCheck(VK_LBUTTON);
	inputKeyCheck(VK_RBUTTON);

	//マウスの移動量取得
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