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
	Camera camera; //カメラ制御クラス

	WNDCLASSEX wcx = {}; //ウィンドウレジスタークラス
	WNDCLASSEX wcx2 = {}; //テキストウィンドウレジスタークラス
	HWND hwnd; //ウィンドウハンドル
	HWND text_hwnd; //サブウィンドウハンドル

	/*-----メンバ関数-----*/
	void Initialize();
	void Update();
	void Terminate();

};