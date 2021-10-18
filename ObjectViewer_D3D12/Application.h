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
	DirectXController DxCon;	//to Controll DirectX
	Input input;	//to Controll Input
	Camera camera;	//to Controll Camera

	HWND mhwnd;	//MainWindow Handle
	HINSTANCE hInst;	//InstanceHandle
	HWND ehwnd;	//EditWindow Handle
	HWND hEdit;	//ComboBox Handle use for Select Object which you want to Load
	HWND hEditTr[9];	//EditBox Handle use for Change Object's transform
	HWND hButton;	//LoadButton Handle use for start Load new Object
	HWND hDrop;	//ComboBox Handle use for Select Object which is exist in virtual World

	bool isLoadObject;	//Check this Application is Loading new Object
	std::string LoadObjPath;	//ObjectPath 

	bool isDeleteObject = false;

	bool isUpdateText = false;	//Check this Application is Changing EditBox's text

	std::vector<std::string> DefaultObjFilePaths;	//Loadable Objects List.

	int objIndex = -1;	//Index of Objects List. this will use for reference which Object's parameter should be changed.

public:
	HRESULT CreateMainWindow(WNDCLASSEX& wcx);	//wcx = WindowRegisterClass for MainWindow
	HRESULT CreateEditWindow(WNDCLASSEX& wcx);	//wcx = WindowRegisterClass for EditWindow
	void DeleteObject();

	HRESULT Initialize(WNDCLASSEX& mwcx, WNDCLASSEX& ewcx);	//mwcx = WindowRegisterClass for MainWindow, ewcx = WindowRegisterClass for EditWindow
	void Update();
	void Terminate();
};