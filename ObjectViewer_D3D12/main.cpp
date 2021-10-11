#pragma once
#include"Application.h"

#ifdef _DEBUG
#include<iostream>
#endif


HRESULT InitWindow(Application &app);
LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

int main() {
	HRESULT hr;
	Application app;
	/*-----ウィンドウ生成-----*/
	hr = InitWindow(app);
	if (FAILED(hr)) {
		std::cout << "Failed to InitWindow\n";
		return 0;
	}

	/*-----アプリケーション初期化-----*/
	app.Initialize(app.hwnd);

	/*-----メッセージループ-----*/
	MSG msg = {};
	while (true) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT) {
			break;
		}

		/*-----アプリケーション更新-----*/
		app.Update();
	}

	//�E�B���h�E�N���X�̍폜
	UnregisterClass(app.wcx.lpszClassName, app.wcx.hInstance);

	/*-----�I������-----*/
	app.Terminate();

	return 0;
}

HRESULT InitWindow(Application &app) {
	//�E�B���h�E�N���X�̓o�^�Ɛ���
	app.wcx.cbSize = sizeof(WNDCLASSEX);
	app.wcx.lpfnWndProc = (WNDPROC)WindowProc;
	app.wcx.lpszClassName = _T("DX12Sample");
	app.wcx.hInstance = GetModuleHandle(0);
	if (!RegisterClassEx(&app.wcx)) {
		return S_FALSE;
	}
	//�E�B���h�E�T�C�Y�̐���
	RECT rc = { 0,0,window_Width,window_Height };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

	//�E�B���h�E�̐���
	app.hwnd = CreateWindow(
		app.wcx.lpszClassName,
		_T("DX12Sample"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		app.wcx.hInstance,
		nullptr
	);

	//�E�B���h�E�̕\��
	ShowWindow(app.hwnd, SW_SHOW);
	return S_OK;
}


/*
キー入力はWindowProcで受け付ける
*/
LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	TCHAR tcStr[128];

	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_KEYDOWN:
		return 0;
	case WM_KEYUP:
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}