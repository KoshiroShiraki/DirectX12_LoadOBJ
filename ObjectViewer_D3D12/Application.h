/*
☆Application管理クラス
*/

#pragma once
#include<Windows.h>
#include<tchar.h>
#include<string>
#include<iostream>
#include"DirectXController.h"
#include"ConstValue.h"
#include"PathController.h"
#include"Input.h"

class Application {
public:
	Application();
	~Application();

public:
	DirectXController DxCon; //DirectX制御クラス
	Input input; //インプット制御クラス

	WNDCLASSEX wcx = {}; //ウィンドウレジスタークラス
	HWND hwnd; //ウィンドウハンドル(ウィンドウ識別)

	/*-----メンバ関数-----*/
	void Initialize(HWND hwnd);
	void Update();
	void Terminate();

};