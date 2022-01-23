#pragma once
#include"BaseWindowController.h"

class ListWindowController : public BaseWindowController {
public:
	ListWindowController(HINSTANCE hInstance, int window_width, int window_height);
	~ListWindowController();

public:
	HWND m_bhwnd[3]; //ボタンウィンドウハンドル
	static const int m_btnCnt = 7;
	int m_btn_width;
	int m_btn_height;
	const char* m_btnName[m_btnCnt] = {
		"new",
		"rename",
		"duplicate",
		"save",
		"delete",
		"save scene",
		"quit",
	};
	int m_btn_offsetX;
	int m_btn_offsetY[m_btnCnt];
	HWND m_chwnd; //コンボボックスウィンドウハンドル
	HWND m_lhwnd[2]; //リストボックスウィンドウハンドル
	static const int m_lbCnt = 2;

	//ウィンドウID
	static const int m_newbtnID = 0;
	static const int m_renamebtnID = 1;
	static const int m_duplicatebtnID = 2;
	static const int m_savebtnID = 3;
	static const int m_deletebtnID = 4;
	static const int m_savescenebtnID = 5;
	static const int m_quitbtnID = 6;
	static const int m_cbID = 7;
	static const int m_lbParentID = 8;
	static const int m_lbChildID = 9;

	bool m_isLoad = false;
	bool m_isDelete = false;
	bool m_isDuplicate = false;
	bool m_isParentChanged = false;
	bool m_isChildChanged = false;
	int m_loadIdx = -1; //現在選択されているロード可能オブジェクトのID
	int m_parentIdx = -1; //現在選択されている親オブジェクトのID
	int m_childIdx = -1; //現在選択されている子オブジェクトのID

	std::vector<std::string> m_loadableFileList;

public :
	HRESULT CreateChildWindow() override;
	HRESULT InitChildWindow();
	LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
