/*
入力処理用関数
基本的にはwindowsプロシージャからの入力メッセージ

WASD -> カメラ位置移動
マウス入力 -> カメラ方向転換
*/
#pragma once
#include<Windows.h>
#include<iostream>

//なぜかキーボードのアルファベットキーに対応したマクロがWindowsAPI出は定義されていないので、使用する分だけ定義しておく
//参照 -> https://docs.microsoft.com/ja-jp/windows/win32/inputdev/virtual-key-codes
#define KEY_W 0x57
#define KEY_A 0x41
#define KEY_S 0x53
#define KEY_D 0x44

//入力状態
enum INPUT_STATE {
	INPUT_PRESSED, //キーが押されている状態
	INPUT_RELEASED, //キーが離されている状態
};

class Input {
public:
	Input();
	~Input();

private:
	char inputKey[256]; //キーが押されている間はinputKey[入力キー] = 0x01,キーが離されている間はinputKey[入力キー] = 0x00
	bool inputEnable; //入力受付フラグ

public:
	void keyPressed(WPARAM param); //キーが押された
	void keyRelesed(WPARAM param); //キーが離された
	void enableInput(bool flag);  //キー入力を受け付けるか
};


