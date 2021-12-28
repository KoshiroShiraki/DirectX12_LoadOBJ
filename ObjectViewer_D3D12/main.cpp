#pragma once
#include"Application.h"

#ifdef _DEBUG
#include<iostream>
#endif

static Application app; //アプリケーション管理クラス

int pSliderPos = 0;
int main() {
	HRESULT hr;

	app.hInst = GetModuleHandle(0); //アプリケーションインスタンスハンドルの取得(コンソール画面を出したいのでWinMainは使わない)

	/*-----アプリケーションの初期化-----*/
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
		/*-----アプリケーションの更新-----*/
		if (FAILED(app.Update())) {
			ErrorMessage("Failed to Update Application");
			break;
		}
	}

	/*-----アプリケーションの終了-----*/
	app.Terminate();

	return 0;
}