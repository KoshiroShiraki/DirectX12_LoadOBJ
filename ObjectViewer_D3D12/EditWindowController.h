#pragma once
#include<DirectXMath.h>
#include"BaseWindowController.h"

using namespace DirectX;

class EditWindowController : public BaseWindowController {
public:
	EditWindowController(HINSTANCE hInstance, int window_width, int window_height);
	~EditWindowController();

public :
	static const int m_editCnt = 26;
	HWND m_ehwnd[m_editCnt]; //エディットボックス
	static const int m_editID = 0; //0~25
	int m_eb_width;
	int m_eb_height;
	int m_eb_offsetX[3];
	int m_eb_offsetY[10];
	const char* m_eb_text[10] = {
		"pos",
		"rot",
		"siz",
		"amb",
		"dif",
		"spe",
		"N",
		"d",
		"light col",
		"light dir",
	};
	float m_edValue[m_editCnt];

	int m_s_offsetX;
	int m_dx = 0;
	int m_dy = 0;
	int m_curID = -1;
	double m_delta;
	bool m_editFlag = false;
	bool m_isFocusEb = false;


public :
	HRESULT CreateChildWindow() override;
	void UpdateEditBoxTransform(XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 siz);
	void UpdateEditBoxMaterial(XMFLOAT4 amb, XMFLOAT4 dif, XMFLOAT4 spe, float N);
	LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};