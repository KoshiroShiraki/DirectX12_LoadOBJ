#pragma once
#include"BaseWindowController.h"

class MainWindowController : public BaseWindowController {
public :
	MainWindowController(HINSTANCE hInstance, int window_width, int window_height);
	~MainWindowController();

	HRESULT CreateChildWindow() override;

	LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};
