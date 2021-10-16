#pragma once
#include"Application.h"

#ifdef _DEBUG
#include<iostream>
#endif

#define EDIT_BOX_ID 1
#define BUTTON_LOAD 2
#define DROPDOWN_BOX 3
#define EDIT_BOX_TRANSFORM 4//~12

HRESULT CreateMainWindowRegister(WNDCLASSEX &wcx, HINSTANCE hInst); //メインウィンドウ用のウィンドウクラス登録
HRESULT CreateEditWindowRegister(WNDCLASSEX& wcx, HINSTANCE hInst); //エディタウィンドウ用のウィンドウクラス登録
LRESULT mainWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
LRESULT editWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

Application app; //アプリケーション制御クラス

int main() {
	HRESULT hr;

	app.hInst = GetModuleHandle(0); //現在のアプリケーションのインスタンスハンドルを取得する

	WNDCLASSEX mwcx = {}; //メインウィンドウ用のウィンドウクラス
	WNDCLASSEX ewcx = {}; //エディタウィンドウ用のウィンドウクラス

	/*-----ウィンドウクラス登録-----*/
	hr = CreateMainWindowRegister(mwcx, app.hInst);
	if (FAILED(hr)) {
		std::cout << "Failed to MainWindowRegister\n";
		return 0;
	}
	hr = CreateEditWindowRegister(ewcx, app.hInst);
	if (FAILED(hr)) {
		std::cout << "Failed to EditWindowRegister\n";
		return 0;
	}

	/*-----アプリケーション初期化-----*/
	app.Initialize(mwcx, ewcx);

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

	UnregisterClass(mwcx.lpszClassName, app.hInst); 
	UnregisterClass(ewcx.lpszClassName, app.hInst);

	/*-----アプリケーション終了-----*/
	app.Terminate();

	return 0;
}

HRESULT CreateMainWindowRegister(WNDCLASSEX& wcx, HINSTANCE hInst) {
	//ウィンドウクラスの登録
	wcx.cbSize = sizeof(WNDCLASSEX);
	wcx.style = CS_VREDRAW | CS_HREDRAW;
	wcx.lpfnWndProc = (WNDPROC)mainWindowProc; //メイン用のウィンドウプロシージャ
	wcx.lpszClassName = _T("ObjectViewer");
	wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcx.hInstance = hInst;
	if (!RegisterClassEx(&wcx)) {
		return S_FALSE;
	}

	return S_OK;
}

HRESULT CreateEditWindowRegister(WNDCLASSEX& wcx, HINSTANCE hInst) {
	//ウィンドウクラスの登録
	wcx.cbSize = sizeof(WNDCLASSEX);
	wcx.style = CS_VREDRAW | CS_HREDRAW | CS_NOCLOSE; //アプリが終了するまでウィンドウは表示/非表示で切り替えるのみで破棄されないためにCLOSEボタンはなし
	wcx.lpfnWndProc = (WNDPROC)editWindowProc; //エディタ用のウィンドウプロシージャ
	wcx.lpszClassName = _T("EditorBox");
	wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcx.hInstance = hInst;
	if (!RegisterClassEx(&wcx)) {
		return S_FALSE;
	}

	return S_OK;
}

//メインウィンドウ(DirectXの描画先)用プロシージャ
LRESULT mainWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

//エディタウィンドウ(オブジェクトを読み込んだりオブジェクトの位置姿勢制御など)用プロシージャ
LRESULT editWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	//ウィンドウの描画(テキストとか)に使うやつ
	PAINTSTRUCT ps;
	HDC hdc;

	switch (msg) {
	case WM_PAINT: //文字描画
		hdc = BeginPaint(hwnd, &ps);

		TextOut(hdc, 5, 5, TEXT("FileName"), 8);
		TextOut(hdc, 5, 45, TEXT("ObjectEditor"), 12);
		TextOut(hdc, 5, 105, TEXT("Position"), 8);
		TextOut(hdc, 5, 135, TEXT("Rotation"), 8);
		TextOut(hdc, 5, 165, TEXT("Size"), 4);

		EndPaint(hwnd, &ps);
		return 0;

	case WM_CREATE:
		//エディターウィンドウに必要なもろもろのセットアップ
		/*-----エディットボックス-----*/
		app.hEdit = CreateWindowEx(
			0,
			"COMBOBOX",
			"",
			WS_CHILD | WS_VISIBLE | CBS_DROPDOWN,
			70,
			5,
			400,
			500,
			hwnd,
			(HMENU)EDIT_BOX_ID,
			app.hInst,
			NULL
		);
		for (int i = 0; i < app.DefaultObjFilePaths.size(); i++) {
			SendMessage(app.hEdit, CB_ADDSTRING, 0, (LPARAM)app.DefaultObjFilePaths[i].c_str());
		}

		/*-----ロードボタン(これが押されたらオブジェクトロード開始)-----*/
		app.hButton = CreateWindowEx(0,
			"BUTTON",
			"Load",
			WS_CHILD | WS_VISIBLE,
			480,
			5,
			50,
			20,
			hwnd,
			(HMENU)BUTTON_LOAD,
			app.hInst,
			NULL
		);

		/*-----ドロップダウンボックス(ロードされて表示されているオブジェクトの列挙)-----*/
		app.hDrop = CreateWindowEx(
			0,
			"COMBOBOX",
			nullptr,
			WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_OVERLAPPED,
			5,
			70,
			500,
			500, //ドロップボックスを開いたときの領域込みで高さ指定しなくてはいけないらしい(→https://oshiete.goo.ne.jp/qa/7318205.html)
			hwnd,
			(HMENU)DROPDOWN_BOX,
			app.hInst,
			nullptr
		);
		/*-----トランスフォーム用エディットボックス(位置姿勢を変える用)-----*/
		/*
		識別ID
		4  5  6
		7  8  9
		10 11 12
		*/
		for (int i = 0; i < 9; i++) {
			app.hEditTr[i] = CreateWindowEx(
				0,
				"EDIT",
				nullptr,
				WS_CHILD | WS_VISIBLE | WS_BORDER,
				80 + 60 * (i % 3),
				105 + 30 * (i / 3),
				50,
				25,
				hwnd,
				(HMENU)(EDIT_BOX_TRANSFORM + i),
				app.hInst,
				nullptr
			);
		}
		return 0;

	case WM_COMMAND:
		switch (LOWORD(wparam)) {
			//ボタンが押されたときの処理

		case BUTTON_LOAD: //ロードボタン
			if (!app.isLoadObject) { //オブジェクトのロードが始まると終了までプログラムが占有されるのでほぼ心配はないが、念のためロードフラグが立っていた場合には押しても何も起こらないようにしておく

				 //テキストボックス内のテキストを取得(オブジェクトのファイル名)
				LPTSTR objPath = (LPTSTR)calloc((GetWindowTextLength(app.hEdit) + 1), sizeof(TCHAR));
				GetWindowText(app.hEdit, objPath, GetWindowTextLength(app.hEdit) + 1);

				app.LoadObjPath = objPath; //読み込むオブジェクトパスを設定して
				app.isLoadObject = true; //オブジェクトのロードを開始する(次のアプリけーションフレーム処理前にロードが行われる)
			}
			return 0;

		case DROPDOWN_BOX: //オブジェクト選択ドロップダウンボックス
			switch (HIWORD(wparam)) {
			case CBN_SELCHANGE: //項目が選択されたとき
				//選択された項目のインデックス番号はそのまま対応するオブジェクト配列のインデックス番号なので
				//インデックス番号を取得しそのオブジェクトのposition,rotation,sizeを取得しエディットボックスにセットする
				//ポジションの設定
				//ここでWM_SETTEXTメッセージを送ると同時にエディットウィンドウでES_UPDATEメッセージも受信してしまうので、フラグを管理しすべてのテキストのセットが終わるまでフラグを下ろさない
				app.isUpdateText = true;
				{
					app.objIndex = SendMessage(app.hDrop, CB_GETCURSEL, 0, 0);
					SendMessage(app.hEditTr[0], WM_SETTEXT, 0, (LPARAM)std::to_string(app.DxCon.objs[app.objIndex].transform.position.x).c_str());
					SendMessage(app.hEditTr[1], WM_SETTEXT, 0, (LPARAM)std::to_string(app.DxCon.objs[app.objIndex].transform.position.y).c_str());
					SendMessage(app.hEditTr[2], WM_SETTEXT, 0, (LPARAM)std::to_string(app.DxCon.objs[app.objIndex].transform.position.z).c_str());
					SendMessage(app.hEditTr[3], WM_SETTEXT, 0, (LPARAM)std::to_string(app.DxCon.objs[app.objIndex].transform.rotation.x).c_str());
					SendMessage(app.hEditTr[4], WM_SETTEXT, 0, (LPARAM)std::to_string(app.DxCon.objs[app.objIndex].transform.rotation.y).c_str());
					SendMessage(app.hEditTr[5], WM_SETTEXT, 0, (LPARAM)std::to_string(app.DxCon.objs[app.objIndex].transform.rotation.z).c_str());
					SendMessage(app.hEditTr[6], WM_SETTEXT, 0, (LPARAM)std::to_string(app.DxCon.objs[app.objIndex].transform.size.x).c_str());
					SendMessage(app.hEditTr[7], WM_SETTEXT, 0, (LPARAM)std::to_string(app.DxCon.objs[app.objIndex].transform.size.y).c_str());
					SendMessage(app.hEditTr[8], WM_SETTEXT, 0, (LPARAM)std::to_string(app.DxCon.objs[app.objIndex].transform.size.z).c_str());
				}
				app.isUpdateText = false;
				return 0;
			}
			return 0;
		}

		//トランスフォーム用エディットボックスのメッセージ受信
		if (app.objIndex != -1 && !app.isUpdateText) {
			for (int i = 0; i < 9; i++) {
				int editNo = EDIT_BOX_TRANSFORM + i;
				if (LOWORD(wparam) == editNo) {
					switch (HIWORD(wparam)) {
					case EN_UPDATE: //エディットボックスの値が変わったとき
						LPTSTR strData[3]; //エディットボックスの文字を取得する文字列
						float data[3]; //エディットボックスには数字が入っているので、その数字をfloatに変換して格納する配列

						switch (i / 3) { //値が 0 = position, 1 = rotation, 2 = size
						case 0:
							app.DxCon.UpdateObjTransform(app.hEditTr, 0, app.DxCon.objs[app.objIndex].transform.position);
							return 0;
						case 1:
							app.DxCon.UpdateObjTransform(app.hEditTr, 3, app.DxCon.objs[app.objIndex].transform.rotation);
							return 0;
						case 2:
							app.DxCon.UpdateObjTransform(app.hEditTr, 6, app.DxCon.objs[app.objIndex].transform.size);
							return 0;
						}
						return 0;
					}
					return 0;
				}
			}
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}