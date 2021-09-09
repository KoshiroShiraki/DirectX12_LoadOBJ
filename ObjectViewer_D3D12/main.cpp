#pragma once
#include"Application.h"

#ifdef _DEBUG
#include<iostream>
#endif

/*-----�v���g�^�C�v�錾-----*/
HRESULT InitWindow();
LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

/*-----�O���[�o���ϐ�------*/
WNDCLASSEX wcx = {};
HWND hwnd;

//�G���g���|�C���g
int main() {
	HRESULT hr;

	/*-----�E�B���h�E����-----*/
	hr = InitWindow();
	if (FAILED(hr)) {
		std::cout << "Failed to InitWindow\n";
		return 0;
	}

	/*-----�A�v���̍쐬-----*/
	Application App;

	/*-----����������-----*/
	App.Initialize(hwnd);

	/*-----���b�Z�[�W���[�v(�Q�[�����[�v)-----*/
	MSG msg = {};
	while (true) {
		/*-----OS���b�Z�[�W����-----*/
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT) {
			break;
		}

		/*-----�Q�[�����[�v����-----*/
		App.Update();
	}

	//�E�B���h�E�N���X�̍폜
	UnregisterClass(wcx.lpszClassName, wcx.hInstance);

	/*-----�I������-----*/
	App.Terminate();

	return 0;
}

HRESULT InitWindow() {
	//�E�B���h�E�N���X�̓o�^�Ɛ���
	wcx.cbSize = sizeof(WNDCLASSEX);
	wcx.lpfnWndProc = (WNDPROC)WindowProc;
	wcx.lpszClassName = _T("DX12Sample");
	wcx.hInstance = GetModuleHandle(0);
	if (!RegisterClassEx(&wcx)) {
		return S_FALSE;
	}
	//�E�B���h�E�T�C�Y�̐���
	RECT rc = { 0,0,window_Width,window_Height };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

	//�E�B���h�E�̐���
	hwnd = CreateWindow(
		wcx.lpszClassName,
		_T("DX12Sample"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		wcx.hInstance,
		nullptr
	);

	//�E�B���h�E�̕\��
	ShowWindow(hwnd, SW_SHOW);
	return S_OK;
}


LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	if (msg == WM_DESTROY) {
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}