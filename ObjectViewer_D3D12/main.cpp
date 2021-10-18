#pragma once
#include"Application.h"

#ifdef _DEBUG
#include<iostream>
#endif

/*this macro value is used for windowID*/
#define EDIT_BOX_ID 1
#define BUTTON_LOAD 2
#define DROPDOWN_BOX 3
#define EDIT_BOX_TRANSFORM 4//~12
#define BUTTON_DELETE 13

HRESULT CreateMainWindowRegister(WNDCLASSEX &wcx, HINSTANCE hInst); //Register Function for MainWindow
HRESULT CreateEditWindowRegister(WNDCLASSEX& wcx, HINSTANCE hInst); //Register Function for EditWindow
LRESULT mainWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam); //WindowProcedure for MainWindow
LRESULT editWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam); //WindowProcedure for EditWindow

Application app; //for Controll this Application

int main() {
	HRESULT hr;

	app.hInst = GetModuleHandle(0); //get current Applicatoin's instance handle

	WNDCLASSEX mwcx = {}; //Register Class for MainWIndow
	WNDCLASSEX ewcx = {}; //Register Class for EditWindow

	/*-----Regist Window Class-----*/
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

	/*-----Initialize Application-----*/
	app.Initialize(mwcx, ewcx);

	/*-----Windows Message Loop-----*/
	MSG msg = {};
	while (true) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT) {
			break;
		}
		/*-----Update Application-----*/
		app.Update();
	}

	UnregisterClass(mwcx.lpszClassName, app.hInst); 
	UnregisterClass(ewcx.lpszClassName, app.hInst);

	/*-----Terminate Application-----*/
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

LRESULT mainWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

/*
EditWindow is used for...
 -to Load New object
 -to Select object present in VirtualWorld
 -to Change object's transform(position, rotation, size) 
*/
LRESULT editWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	//these are used for painting  window
	PAINTSTRUCT ps;
	HDC hdc;

	switch (msg) {
	case WM_PAINT: //Text Initialize
		hdc = BeginPaint(hwnd, &ps);

		TextOut(hdc, 5, 5, TEXT("FileName"), 8);
		TextOut(hdc, 5, 45, TEXT("ObjectEditor"), 12);
		TextOut(hdc, 5, 105, TEXT("Position"), 8);
		TextOut(hdc, 5, 135, TEXT("Rotation"), 8);
		TextOut(hdc, 5, 165, TEXT("Size"), 4);

		EndPaint(hwnd, &ps);
		return 0;

	case WM_CREATE:
		/*
		this message is called only once when new window is created.
		this is used to Generate and Initialize Window needed for EditWindow.
		*/

		/*-----ComboBox1 ... List Loadable Object-----*/
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
		//List Loadable Object when Program Started
		for (int i = 0; i < app.DefaultObjFilePaths.size(); i++) {
			SendMessage(app.hEdit, CB_ADDSTRING, 0, (LPARAM)app.DefaultObjFilePaths[i].c_str());
		}

		/*-----LoadButton ... Load New Object which selected in ConboBox1-----*/
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

		/*-----DeleteButton ... delete exist Object*/
		app.hButton = CreateWindowEx(0,
			"BUTTON",
			"Delete",
			WS_CHILD | WS_VISIBLE,
			480,
			165,
			50,
			20,
			hwnd,
			(HMENU)BUTTON_DELETE,
			app.hInst,
			NULL
		);

		/*-----ComboBox2 ... List Loaded Object-----*/
		app.hDrop = CreateWindowEx(
			0,
			"COMBOBOX",
			nullptr,
			WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_OVERLAPPED,
			5,
			70,
			500,
			500,
			hwnd,
			(HMENU)DROPDOWN_BOX,
			app.hInst,
			nullptr
		);
		/*-----EditBox ... Change Object's transform(position, rotation, size)-----*/
		/*
		Create 9 Edit Box
		--------------------------------
		ID
		4  5  6  ... pos.x, pos.y, pos.z
		7  8  9  ... rot.x, rot.y, rot.z
		10 11 12 ... siz.x, siz.y, siz.z
		--------------------------------
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
		/*
		this message is called when something happened in Window.(e.g. User's Mouse Cursel clicked window)
		WPARAM wparam is 32-bit variable, we can get WindowID from Low 16-bit and can get Message from High 16-bit.
		*/
		switch (LOWORD(wparam)) {
		case BUTTON_LOAD: //LoadButton
			if (!app.isLoadObject) { //Check this App is not Loading new Object.

				 //Get the Text from ComboBox(this text is Object File Path)
				LPTSTR objPath = (LPTSTR)calloc((GetWindowTextLength(app.hEdit) + 1), sizeof(TCHAR));
				GetWindowText(app.hEdit, objPath, GetWindowTextLength(app.hEdit) + 1);

				app.LoadObjPath = objPath; //Set Object File path,
				app.isLoadObject = true; //and Start Loading Object
			}
			return 0;
		case BUTTON_DELETE:
			if (app.objIndex != -1) {
				app.isDeleteObject = true;
			}

		case DROPDOWN_BOX: //ComboBox2
			switch (HIWORD(wparam)) {
			case CBN_SELCHANGE: //when user select Object from Objects-List
				/*
				Return value from SendMessage(hwnd, CB_GETCURSEL, 0, 0) is Index number of Objects List.
				use this index to get Object's transform Parameter.
				*/
				app.isUpdateText = true; 
				//This Application is Get new Object's parameters when EditBox updated, but this process is not Updating Parameters.(if update parameters in this process, this program crash)
				//not to update Parameters, using flag
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

		if (app.objIndex != -1 && !app.isUpdateText) { //EditBox
			for (int i = 0; i < 9; i++) {
				int editNo = EDIT_BOX_TRANSFORM + i; //check EditBox message(from Edit_BOX_TRANSFORM = 4 to 12)
				if (LOWORD(wparam) == editNo) {
					switch (HIWORD(wparam)) {
						//Object' transform is 3 parameters makes a set, so this process 3 params at one loop
					case EN_UPDATE: //if EditBox's text change
						LPTSTR strData[3]; //string variable to get EditBox's text
						float data[3]; //EditBox's text must be Number.(if not so, a parameter will set 0.0f)
						switch (i / 3) { //i = 0~2 is pos, i = 3=5 is rot, i = 6~8 is siz.
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