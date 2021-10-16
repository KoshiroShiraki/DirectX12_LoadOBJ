#pragma once
#include"Application.h"

Application::Application() {
	isLoadObject = false;
}

Application::~Application() {

}

HRESULT Application::CreateMainWindow(WNDCLASSEX &wcx) {
	//Make Window Size Using RECT(WindowsAPI)
	RECT rc = { 0,0,window_Width,window_Height };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

	//ウィンドウ生成
	mhwnd = CreateWindow(
		wcx.lpszClassName,
		_T("Object_Viewer"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		hInst,
		nullptr
	);

	return S_OK;
}

HRESULT Application::CreateEditWindow(WNDCLASSEX &wcx) {
	//Make Window Size Using RECT(WindowsAPI)
	RECT rc = { 0,0,600,200 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);
	ehwnd = CreateWindow(
		wcx.lpszClassName,
		_T("Editor"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		hInst,
		nullptr
	);

	return S_OK;
}

HRESULT Application::Initialize(WNDCLASSEX &mwcx, WNDCLASSEX &ewcx) {
	HRESULT hr;

	/*-----make a List of the Objects which Loadable at Program start-----*/
	PathController pc; //Controller for Path
	char objsPath[MAX_PATH_LENGTH];

	pc.AddLeafPath(pc.basePath, objsPath, "\\ObjectViewer_D3D12\\Model\\OBJ\\*.obj");

	//How to serch Files from local directory→http://nienie.com/~masapico/api_FindFirstFile.html
	HANDLE hFind;
	WIN32_FIND_DATA fd;
	hFind = FindFirstFile(objsPath, &fd);
	if (hFind == INVALID_HANDLE_VALUE) {
	}
	else {
		char objFilesPath[MAX_PATH_LENGTH];
		pc.AddLeafPath("\\ObjectViewer_D3D12\\Model\\OBJ\\", objFilesPath, fd.cFileName); //Create Object File Path for program
		DefaultObjFilePaths.push_back(objFilesPath);
		while (FindNextFile(hFind, &fd)) {
			pc.AddLeafPath("\\ObjectViewer_D3D12\\Model\\OBJ\\", objFilesPath, fd.cFileName);
			DefaultObjFilePaths.push_back(objFilesPath);
		}
	}

	/*-----Generate Window-----*/
	hr = CreateMainWindow(mwcx);
	if (FAILED(hr)) {
		std::cout << "Failed to CreateMainWindow\n";
		return hr;
	}
	hr = CreateEditWindow(ewcx);
	if (FAILED(hr)) {
		std::cout << "Failed to CreateEditWindow" << std::endl;
		return hr;
	}

	/*-----Display Window-----*/
	ShowWindow(mhwnd, SW_SHOW);
	ShowWindow(ehwnd, SW_SHOW);

	/*-----Initialize DirectX-----*/
	if (FAILED(DxCon.InitD3D(mhwnd))) {
		std::cout << "Failed to InitD3D\n";
		return E_FAIL;
	}
	if (FAILED(DxCon.CreateResources(camera))) {
		std::cout << "Failed to CreateResources\n";
		return E_FAIL;
	}
	if (FAILED(DxCon.CreateShaders())) {
		std::cout << "Failed to CreateShaders\n";
		return E_FAIL;
	}
	if (FAILED(DxCon.SetGraphicsPipeLine())) {
		std::cout << "Failed to SetGraphicsPipeLine\n";
		return E_FAIL;
	}

}

void Application::Update() {
	//1.Update Input
	input.update();

	//2.Load New Object(if Application need to Load a New Object)
	if (isLoadObject) {
		std::cout << LoadObjPath << std::endl;
		if ((SUCCEEDED(DxCon.LoadObject(LoadObjPath.c_str())))) { //if suceeded to load New Object,
			//Send Message to ComboBox2 to Add New Object
			PathController pc;
			char objName[256];
			pc.GetLeafDirectryName(LoadObjPath.c_str(), objName, 256);

			SendMessage(hDrop, CB_ADDSTRING, 0, (LPARAM)objName);
		}
		isLoadObject = false; //flag down
	}

	//3.Update Camera
	camera.update(XMFLOAT3(input.inputKey[KEY_D] * (-1) + input.inputKey[KEY_A], input.inputKey[VK_LSHIFT] * (input.inputKey[KEY_W] * (-1) + input.inputKey[KEY_S]), (1 - input.inputKey[VK_LSHIFT]) * (input.inputKey[KEY_W] * (-1) + input.inputKey[KEY_S])), XMFLOAT3(0, input.dPos.x, input.dPos.y), input.inputKey[VK_RBUTTON]);

	//Update-Object-Parameter-Function is called from WindowProcedure when receive message, 
	//and setting new parameter to Object is done in Draw function, so do not need to place Update-Object-Parameter-Function here.

	if (FAILED(DxCon.Draw(camera))) {
		std::cout << "Failed to Update\n";
		return;
	}
}

void Application::Terminate() {
	/*-----Rerlease DirectX Interface-----*/
	DxCon.Release();
}