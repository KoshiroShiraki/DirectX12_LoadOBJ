#pragma once
#include"Application.h"

#ifdef _DEBUG
#include<iostream>
#endif


HRESULT InitWindow();
LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

Application app;

int main() {
	HRESULT hr;
	/*-----ウィンドウ生成-----*/
	hr = InitWindow();
	if (FAILED(hr)) {
		std::cout << "Failed to InitWindow\n";
		return 0;
	}

	/*-----アプリケーション初期化-----*/
	app.Initialize();

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

	//ウィンドウクラスレジスタの解除
	UnregisterClass(app.wcx.lpszClassName, app.wcx.hInstance);

	/*-----アプリケーション終了-----*/
	app.Terminate();

	return 0;
}

HRESULT InitWindow() {
	//ウィンドウクラスの登録
	app.wcx.cbSize = sizeof(WNDCLASSEX);
	app.wcx.lpfnWndProc = (WNDPROC)WindowProc;
	app.wcx.lpszClassName = _T("DX12Sample");
	app.wcx.hInstance = GetModuleHandle(0);
	if (!RegisterClassEx(&app.wcx)) {
		return S_FALSE;
	}
	//ウィンドウサイズ
	RECT rc = { 0,0,window_Width,window_Height };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

	//ウィンドウ生成
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

	//ウィンドウ表示
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
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}