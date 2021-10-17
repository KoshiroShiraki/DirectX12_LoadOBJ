#include"Input.h"

Input::Input() {
	ZeroMemory(inputKey, sizeof(inputKey));
}

Input::~Input() {

}

void Input::update() {
	//Check WASD Input
	inputKeyCheck(KEY_W);
	inputKeyCheck(KEY_A);
	inputKeyCheck(KEY_S);
	inputKeyCheck(KEY_D);

	//Check LeftShift Input
	inputKeyCheck(VK_LSHIFT);

	//Check Mouse Input
	inputKeyCheck(VK_LBUTTON);
	inputKeyCheck(VK_RBUTTON);

	//get Mouse movement value
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