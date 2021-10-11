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

class Application {
public:
	/*-----メンバ変数-----*/
	DirectXController DxCon;
	
	/*-----コンストラクタ/デストラクタ-----*/
	Application();
	~Application();

	/*-----メンバ関数-----*/
	void Initialize(HWND hwnd);
	void Update();
	void Terminate();

};