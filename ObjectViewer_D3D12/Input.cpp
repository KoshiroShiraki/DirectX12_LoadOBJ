#include"Input.h"

Input::Input() {
	//キー入力用配列の中身を0にする
	ZeroMemory(inputKey, sizeof(inputKey));
}

Input::~Input() {

}

void Input::keyPressed(WPARAM param) {
	inputKey[param] = 0x01;
	switch(param) {
	case KEY_W:
		std::cout << "W" << std::endl;
		break;
	case KEY_A:
		std::cout << "A" << std::endl;
		break;
	case KEY_S:
		std::cout << "S" << std::endl;
		break;
	case KEY_D:
		std::cout << "D" << std::endl;
		break;
	}
}

void Input::keyRelesed(WPARAM param) {
	inputKey[param] = 0x00;
}