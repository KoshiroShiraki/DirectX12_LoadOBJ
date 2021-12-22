#pragma once
#include<Windows.h>
#include<iostream>
#include<vector>
#include<string>
#include"ErrorDebug.h"
#include"PathController.h"

class BaseWindowController {
public:
	HWND m_hwnd; //ウィンドウハンドル
	WNDCLASSEX m_wcx; //ウィンドウクラス定義

protected:
	ATOM m_atom; //ウィンドウ登録チェック用
	HINSTANCE m_hinst; //アプリケーションインスタンスハンドル

public:
	int m_window_width; //ウィンドウ幅
	int m_window_height; //ウィンドウ高さ

	int m_client_width; //クライアント領域幅
	int m_client_height; //クライアント領域高さ

public:
	BaseWindowController(HINSTANCE hInstance, int window_width, int window_height);
	~BaseWindowController();

public:
	HRESULT InitWindow(LPCSTR className, LPCSTR windowName); //ウィンドウ初期化(ウィンドウクラス登録 -> メインウィンドウ生成 -> 子ウィンドウ生成)
	virtual HRESULT CreateChildWindow() = 0;

protected:
	HRESULT CreateMainWindow(LPCSTR className, LPCSTR windowName); //継承クラスはそれぞれ必ずm_hwndハンドルに紐づいた一つのメインウィンドウを持つ
	HRESULT CreateButton(HWND& hBtn, LPCSTR text, int offsetX, int offsetY, int width, int height, int btnID); //ボタンを生成する
	HRESULT CreateDropDownComboBox(HWND& hCb, LPCSTR text, int offsetX, int offsetY, int width, int height, int cbID); //コンボボックスを生成する
	HRESULT CreateListBox(HWND& hLb, LPCSTR text, int offsetX, int offsetY, int width, int height, int lbID); //リストボックスを生成する
	HRESULT CreateEditBox(HWND& hEb, LPCSTR text, int offsetX, int offsetY, int width, int height, int ebID);
	HRESULT CreateSlider(HWND& hSb, LPCSTR text, int offsetX, int offsetY, int width, int height, int sID);
	void DeleteWindow(); //ウィンドウの削除とクラスの登録解除

public:
	virtual LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0; //継承クラスごとの実装を実際に記述するウィンドウプロシージャ
	static LRESULT CALLBACK baseWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam); //ウィンドウクラスに設定する用のウィンドウプロシージャ
};