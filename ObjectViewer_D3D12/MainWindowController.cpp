#pragma once 
#include"MainWindowController.h"

MainWindowController::MainWindowController(HINSTANCE hInstance, int window_width, int window_height) : BaseWindowController(hInstance, window_width, window_height) {

}

HRESULT MainWindowController::CreateChildWindow() {
	//Žg‚í‚È‚¢
	return S_OK;
}

LRESULT MainWindowController::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_QUIT) {
		PostQuitMessage(0);
		return 0;
	}
	else return DefWindowProc(hwnd, uMsg, wParam, lParam);
}