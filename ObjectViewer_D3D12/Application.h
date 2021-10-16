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

	HWND hwnd; //ウィンドウハンドル
	HINSTANCE hInst; //インスタンスハンドル
	HWND text_hwnd; //サブウィンドウハンドル
	HWND hEdit; //エディットボックス
	HWND hEditTr[9]; //トランスフォーム用エディットボックス
	HWND hButton; //ボタン
	HWND hDrop; //ドロップダウンボックス

	bool isLoadObject; //オブジェクトロードフラグ
	std::string LoadObjPath; //ロードオブジェクトのパス

	bool isUpdateText = false;

	std::vector<std::string> DefaultObjFilePaths;

	int objIndex = -1; //エディタウィンドウからトランスフォームを変更する時に参照するインデックス(デフォルト値は-1)

	/*-----メンバ関数-----*/
	HRESULT CreateMainWindow(WNDCLASSEX& wcx);
	HRESULT CreateEditWindow(WNDCLASSEX& wcx);

	HRESULT Initialize(WNDCLASSEX& mwcx, WNDCLASSEX& ewcx);
	void Update();
	void Terminate();
};