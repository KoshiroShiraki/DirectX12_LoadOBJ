#pragma once
#include<Windows.h>
#include<iostream>
#include<DirectXMath.h>

using namespace DirectX;

//Key Reference -> https://docs.microsoft.com/ja-jp/windows/win32/inputdev/virtual-key-codes
#define KEY_W 0x57
#define KEY_A 0x41
#define KEY_S 0x53
#define KEY_D 0x44
#define KEY_E 0x45

class Input {
public:
	Input();
	~Input();

public:
	char inputKey[256]; //Key Pressed -> inputKey[x] = 1, Key Released -> inputKey[x] = 0
	bool inputEnable;
	bool cameraRotateEnable;

	POINT startPos; //Mouse cursor Position(start)
	POINT endPos; //Mosue Cursor Position(end)
	POINT dPos; //amount of Change Mouse Cursor Position
	
private:
	/*
	WindosAPI's Check Key Input, Key Pressed Flag is "value of 32-bit data's highest bit is 1"
	KeyReleased Flag is "value of 32-bit data's highest bit is 0"
	so, we can check input with (32-bit data) & (0x8000)
	*/
	const int checkKey = 0x8000; 

public:
	void update();
	void inputKeyCheck(WPARAM param);	//param = return value from Windows Procedure
};


