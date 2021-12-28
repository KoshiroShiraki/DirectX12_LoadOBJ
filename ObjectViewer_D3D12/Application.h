#pragma once
#include<Windows.h>
#include<tchar.h>
#include<string>
#include<iostream>
#include"DirectXController.h"
#include"ConstValue.h"
#include"PathController.h"
#include"Input.h"
#include"MainWindowController.h"
#include"ListWindowController.h"
#include"EditWindowController.h"

class Application {
public:
	Application();
	~Application();

public:
	DirectXController DxCon; //描画用(DirectX12)
	Input input; //入力用
	Camera camera; //カメラ
	Light light; //ライト

	MainWindowController* m_mwc = nullptr; //メインウィンドウ(描画結果を表示)
	ListWindowController* m_lwc = nullptr; //リストウィンドウ(3Dモデルを管理)
	EditWindowController* m_ewc = nullptr; //エディットウィンドウ(描画パラメータの管理)

	HINSTANCE hInst;	//InstanceHandle
	HWND ehwnd;	//EditWindow Handle
	HWND hEdit;	//ComboBox Handle use for Select Object which you want to Load
	HWND hEditTr[9];	//EditBox Handle use for Change Object's transform
	HWND hButton;	//LoadButton Handle use for start Load new Object
	HWND hDrop;	//ComboBox Handle use for Select Object which is exist in virtual World
	HWND hSlider[9]; //Slider Handle use for Change Object's transform
	SCROLLINFO scrInfo[9];

	bool isLoadObject;	//Check this Application is Loading new Object
	std::string LoadObjPath;	//ObjectPath 

	bool isDeleteObject = false;

	std::vector<std::string> DefaultObjFilePaths;	//Loadable Objects List.

	int objIndex = -1;	//Index of Objects List. this will use for reference which Object's parameter should be changed.

public:
	void DeleteObject();
	void UpdateListBox();
	void UpdateEditBox();
	void UpdateParentListBox();
	void UpdateChildListBox();

	HRESULT Initialize();	//mwcx = WindowRegisterClass for MainWindow, ewcx = WindowRegisterClass for EditWindow
	void Update();
	void Terminate();
};