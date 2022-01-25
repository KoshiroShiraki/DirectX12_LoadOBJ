#pragma once
#include"BaseWindowController.h"

class RenameWindowController : public BaseWindowController {
public:
	RenameWindowController(HINSTANCE hInstance, int window_width, int window_height);
	~RenameWindowController();

public:
	//�G�f�B�b�g�{�b�N�X
	HWND m_ehwnd;
	int m_eb_width;
	int m_eb_height = 25;

	//�{�^��
	HWND m_bhwnd;
	int m_btn_width = 50;
	int m_btn_height = 25;
	static const int m_okbtnID = 1;

	bool m_renameCompleted = false; //���l�[�������t���O

	LPTSTR name;

public:
	HRESULT CreateChildWindow() override;
	LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};