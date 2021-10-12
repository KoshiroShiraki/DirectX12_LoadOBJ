#pragma once
#include"Application.h"

Application::Application() {

}

Application::~Application() {

}

void Application::Initialize() {
	HANDLE hFind;
	WIN32_FIND_DATA fd;
	FILETIME ft;
	SYSTEMTIME st;

	/*-----DirectX初期化-----*/
	if (FAILED(DxCon.InitD3D(hwnd))) {
		std::cout << "Failed to InitD3D\n";
		return;
	}
	if (FAILED(DxCon.CreateResources(camera))) {
		std::cout << "Failed to CreateResources\n";
		return;
	}
	if (FAILED(DxCon.CreateShaders())) {
		std::cout << "Failed to CreateShaders\n";
		return;
	}
	if (FAILED(DxCon.SetGraphicsPipeLine())) {
		std::cout << "Failed to SetGraphicsPipeLine\n";
		return;
	}

}

void Application::Update() {
	//入力情報のアップデート
	//入力を反映する更新処理は必ずこの後に持ってくる
	input.update();

	//カメラのアップデート
	//WとSは左SHIFT入力時に前後ではなく上下方向への移動になる
	camera.update(XMFLOAT3(input.inputKey[KEY_D] * (-1) + input.inputKey[KEY_A], input.inputKey[VK_LSHIFT] * (input.inputKey[KEY_W] * (-1) + input.inputKey[KEY_S]), (1 - input.inputKey[VK_LSHIFT]) * (input.inputKey[KEY_W] * (-1) + input.inputKey[KEY_S])), XMFLOAT3(0, input.dPos.x, input.dPos.y), input.inputKey[VK_RBUTTON]);
	
	//定数バッファのアップデート
	DxCon.UpdateConstantBuffer(camera);
	if (FAILED(DxCon.Draw())) { //描画に失敗したらアプリ終了
		std::cout << "Failed to Update\n";
		return;
	}
}

void Application::Terminate() {
	/*-----DirectXインターフェースの開放-----*/
	DxCon.Release();
}