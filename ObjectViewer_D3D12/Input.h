/*
入力処理用関数
基本的にはwindowsプロシージャからの入力メッセージ

WASD -> カメラ位置移動
マウス入力 -> カメラ方向転換
*/
#pragma once
#include<Windows.h>
#include<iostream>
#include<DirectXMath.h>

using namespace DirectX;

//なぜかキーボードのアルファベットキーに対応したマクロがWindowsAPI出は定義されていないので、使用する分だけ定義しておく
//参照 -> https://docs.microsoft.com/ja-jp/windows/win32/inputdev/virtual-key-codes
#define KEY_W 0x57
#define KEY_A 0x41
#define KEY_S 0x53
#define KEY_D 0x44
#define KEY_E 0x45

//入力状態
enum INPUT_STATE {
	INPUT_PRESSED, //キーが押されている状態
	INPUT_RELEASED, //キーが離されている状態
};

class Input {
public:
	Input();
	~Input();

public:
	char inputKey[256]; //キーが押されている間はinputKey[入力キー] = 0x01,キーが離されている間はinputKey[入力キー] = 0x00
	bool inputEnable; //入力受付フラグ
	bool cameraRotateEnable; //右クリックされている間は、カメラ方向操作を受け付ける

	POINT startPos; //1フレーム前のマウスカーソル座標(視点)
	POINT endPos; //現在フレームのマウスカーソル座標(終点)
	POINT dPos; //座標の変化量
	
private:
	const int checkKey = 0x8000; //32ビットの最上位ビットが1 = 押されているなのでその判定用に使う

public:
	void update();
	void inputKeyCheck(WPARAM param);
	void inputMouseCheck(WPARAM param);

	void MouseLeftButtonDown(bool flag); //マウスが右クリックされている
};


