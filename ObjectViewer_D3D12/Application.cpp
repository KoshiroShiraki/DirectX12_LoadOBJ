#pragma once
#include"Application.h"

Application::Application() {
	isLoadObject = false;
}

Application::~Application() {

}

HRESULT Application::CreateMainWindow(WNDCLASSEX &wcx) {
	//ウィンドウサイズ
	RECT rc = { 0,0,window_Width,window_Height };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

	//ウィンドウ生成
	hwnd = CreateWindow(
		wcx.lpszClassName,
		_T("DX12Sample"),
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
	//テキストボックス配置用のウィンドウの生成
	RECT rc = { 0,0,600,400 };
	text_hwnd = CreateWindow(
		wcx.lpszClassName,
		_T("EditBox"),
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

	/*-----表示可能なオブジェクトの列挙(\Model\OBJフォルダ直下にメッシュファイル、マテリアルファイルを直置きすること)-----*/
	PathController pc; //パスコントローラ
	char objsPath[MAX_PATH_LENGTH];
	pc.AddLeafPath(pc.basePath, objsPath, "\\ObjectViewer_D3D12\\Model\\OBJ\\*.obj");
	std::cout << objsPath << std::endl;
	HANDLE hFind;
	WIN32_FIND_DATA fd;

	//ファイル検索→http://nienie.com/~masapico/api_FindFirstFile.html
	hFind = FindFirstFile(objsPath, &fd);
	if (hFind == INVALID_HANDLE_VALUE) {
	}
	else {
		char objFilesPath[MAX_PATH_LENGTH];
		pc.AddLeafPath("\\ObjectViewer_D3D12\\Model\\OBJ\\", objFilesPath, fd.cFileName); //それに取得したファイル名をくっつける
		DefaultObjFilePaths.push_back(objFilesPath); //プッシュ
		while (FindNextFile(hFind, &fd)) {
			pc.AddLeafPath("\\ObjectViewer_D3D12\\Model\\OBJ\\", objFilesPath, fd.cFileName);
			DefaultObjFilePaths.push_back(objFilesPath); //プッシュ
		}
	}

	/*-----ウィンドウ生成-----*/
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

	/*-----ウィンドウ表示-----*/
	ShowWindow(hwnd, SW_SHOW);
	ShowWindow(text_hwnd, SW_SHOW);

	/*-----DirectX初期化-----*/
	if (FAILED(DxCon.InitD3D(hwnd))) {
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
	//入力情報のアップデート
	//入力を反映する更新処理は必ずこの後に持ってくる
	input.update();

	//オブジェクト読み込み
	if (isLoadObject) {
		std::cout << LoadObjPath << std::endl;
		if ((SUCCEEDED(DxCon.LoadObject(LoadObjPath.c_str())))) { //読み込みに成功したら
			//ドロップボックスに要素追加のメッセージを送る
			PathController pc;
			char objName[256];
			pc.GetLeafDirectryName(LoadObjPath.c_str(), objName, 256);

			SendMessage(hDrop, CB_ADDSTRING, 0, (LPARAM)objName);
		}
		isLoadObject = false; //フラグを下げる
	}

	//カメラのアップデート
	//WとSは左SHIFT入力時に前後ではなく上下方向への移動になる
	camera.update(XMFLOAT3(input.inputKey[KEY_D] * (-1) + input.inputKey[KEY_A], input.inputKey[VK_LSHIFT] * (input.inputKey[KEY_W] * (-1) + input.inputKey[KEY_S]), (1 - input.inputKey[VK_LSHIFT]) * (input.inputKey[KEY_W] * (-1) + input.inputKey[KEY_S])), XMFLOAT3(0, input.dPos.x, input.dPos.y), input.inputKey[VK_RBUTTON]);
	DxCon.UpdateViewMatrix(camera);

	//ワールド行列のアップデート
	for (int i = 0; i < DxCon.LoadedObjCount; i++) {
		DxCon.UpdateWorldMatrix(DxCon.objs[i]);
	}

	if (FAILED(DxCon.Draw())) { //描画に失敗したらアプリ終了
		std::cout << "Failed to Update\n";
		return;
	}
}

void Application::Terminate() {
	/*-----DirectXインターフェースの開放-----*/
	DxCon.Release();
}