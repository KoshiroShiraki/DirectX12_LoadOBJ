#pragma once
#include"Application.h"

Application::Application() {

}

Application::~Application() {

}

void Application::Initialize(HWND hwnd) {
	/*-----DirectX初期化-----*/
	if (FAILED(DxCon.InitD3D(hwnd))) {
		std::cout << "Failed to InitD3D\n";
		return;
	}
	if (FAILED(DxCon.CreateResources())) {
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
	if (FAILED(DxCon.Draw())) {
		std::cout << "Failed to Update\n";
		return;
	}
}

void Application::Terminate() {
	/*-----DirectXインターフェースの開放-----*/
	DxCon.Release();
}