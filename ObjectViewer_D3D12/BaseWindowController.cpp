#pragma once
#include"BaseWindowController.h"

BaseWindowController::BaseWindowController(HINSTANCE hInstance, int window_width, int window_height) {
	m_hinst = hInstance;

	m_window_width = window_width;
	m_window_height = window_height;
}

BaseWindowController::~BaseWindowController() {
	DeleteWindow();
}

HRESULT BaseWindowController::InitWindow(LPCSTR className, LPCSTR windowName) {
	if (FAILED(CreateMainWindow(className, windowName))) {
		return ErrorMessage("Failed to Create MainWindow");
	}
	if (FAILED(CreateChildWindow())) {
		return ErrorMessage("Failed to Create ChildWindow");
	}

	return S_OK;
}

HRESULT BaseWindowController::CreateMainWindow(LPCSTR className, LPCSTR windowName) {
	//�E�B���h�E�o�^
	m_wcx.cbSize = sizeof(WNDCLASSEX);
	m_wcx.style = CS_HREDRAW | CS_VREDRAW;
	m_wcx.lpfnWndProc = BaseWindowController::baseWindowProc;
	m_wcx.cbClsExtra = 0;
	m_wcx.cbWndExtra = 0;
	m_wcx.hInstance = m_hinst;
	m_wcx.hIcon = NULL;
	m_wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
	m_wcx.hbrBackground = NULL;
	m_wcx.lpszMenuName = NULL;
	m_wcx.lpszClassName = className;
	m_wcx.hIconSm = NULL;

	//�E�B���h�E�o�^
	m_atom = RegisterClassEx(&m_wcx);
	if (!m_atom) {
		return ErrorMessage("Failed to RegisterClassEX : MainWindow");
	}

	//�E�B���h�E�쐬
	m_hwnd = CreateWindow(
		m_wcx.lpszClassName,
		windowName,
		WS_OVERLAPPED,
		CW_USEDEFAULT, CW_USEDEFAULT,
		m_window_width, m_window_height,
		NULL,
		NULL,
		m_wcx.hInstance,
		this //�d�v : �����ɃN���X�I�u�W�F�N�g�|�C���^��n�����Ƃ�CREATESTRUCT����E�B���h�E�ƃN���X�I�u�W�F�N�g�̕R�Â����\�ɂȂ�
	);

	if (!m_hwnd) {
		return ErrorMessage("Failed to CreateWindow : TitleWindow");
	}

	//�E�B���h�E�̐����ɐ���������A�N���C�A���g�̈���擾
	RECT rc = {};
	if (!GetClientRect(m_hwnd, &rc)) {
		return ErrorMessage("Failed to GetClientRect");
	}
	m_client_width = rc.right - rc.left;
	m_client_height = rc.bottom - rc.top;


	return S_OK;
}

HRESULT BaseWindowController::CreateButton(HWND& hbtn, LPCSTR text, int offsetX, int offsetY, int width, int height, int btnID) {
	hbtn = CreateWindow(
		"BUTTON",
		text,
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		offsetX, offsetY,
		width, height,
		m_hwnd,
		(HMENU)btnID,
		m_hinst,
		NULL
	);
	if (!hbtn) {
		return ErrorMessage("Failed to Create Button");
	}
	return S_OK;
}

HRESULT BaseWindowController::CreateDropDownComboBox(HWND& hCb, LPCSTR text, int offsetX, int offsetY, int width, int height, int cbID) {
	hCb = CreateWindow(
		"COMBOBOX",
		text,
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWN,
		offsetX, offsetY,
		width, height,
		m_hwnd,
		(HMENU)cbID,
		m_hinst,
		NULL
	);
	if (!hCb) {
		return ErrorMessage("Failed to Create DropDownComboBox");
	}
	return S_OK;
}

HRESULT BaseWindowController::CreateListBox(HWND& hLb, LPCSTR text, int offsetX, int offsetY, int width, int height, int lbID) {
	hLb = CreateWindow(
		"LISTBOX",
		text,
		WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
		offsetX, offsetY,
		width, height,
		m_hwnd,
		(HMENU)lbID,
		m_hinst,
		NULL
	);
	if (!hLb) {
		return ErrorMessage("Failed to Create ListBox");
	}

	return S_OK;
}

HRESULT BaseWindowController::CreateEditBox(HWND& hEb, LPCSTR text, int offsetX, int offsetY, int width, int height, int ebID) {
	hEb = CreateWindow(
		"EDIT",
		text,
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
		offsetX, offsetY,
		width, height,
		m_hwnd,
		(HMENU)ebID,
		m_hinst,
		NULL
	);
	if (!hEb) {
		return ErrorMessage("Failed to Create EditBox");
	}

	return S_OK;
}

HRESULT BaseWindowController::CreateSlider(HWND& hSb, LPCSTR text, int offsetX, int offsetY, int width, int height, int sID){
	hSb = CreateWindow(
		"SCROLLBAR",
		text,
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
		offsetX, offsetY,
		width, height,
		m_hwnd,
		(HMENU)sID,
		m_hinst,
		NULL
	);
	if (!hSb) {
		return ErrorMessage("Failed to Create Slider");
	}

	return S_OK;
}

void BaseWindowController::DeleteWindow() {
	//�E�B���h�E��j�����A�N���X�o�^����������

	if (m_hwnd) { //�E�B���h�E�����݂���ꍇ
		DestroyWindow(m_hwnd);
		m_hwnd = 0;
	}

	if (m_atom) { //�E�B���h�E�N���X���o�^����Ă���ꍇ
		UnregisterClass(m_wcx.lpszClassName, m_hinst);
		m_atom = 0;
	}
}

//�Q�l -> https://suzulang.com/win32api%E3%81%A7%E3%82%A6%E3%82%A3%E3%83%B3%E3%83%89%E3%82%A6%E3%82%92%E3%82%AB%E3%83%97%E3%82%BB%E3%83%AB%E5%8C%96/
LRESULT CALLBACK BaseWindowController::baseWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	BaseWindowController* This = (BaseWindowController*)GetWindowLongPtr(hwnd, GWLP_USERDATA); //�E�B���h�E�ɕR�Â����Ă���C���X�^���X�|�C���^�̎擾

	if (!This) { //�擾���ł��Ȃ� -> �R�Â����Ă��Ȃ�
		if (uMsg == WM_CREATE) { //�E�B���h�E�������ɁA�N���X�C���X�^���X�ƃE�B���h�E�n���h����R�Â���
			This = (BaseWindowController*)((LPCREATESTRUCT)lParam)->lpCreateParams;
			if (This) {
				SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)This); //�E�B���h�E�̃��[�U�f�[�^�ɃC���X�^���X�|�C���^���Z�b�g����
				return This->WindowProc(hwnd, uMsg, wParam, lParam);
			}
			else { //lpCreateParams��0 -> CreateWindow�����s���Ă���A�E�B���h�E�v���V�[�W�����@�\���Ȃ��Ȃ�̂Ńv���O�������I��������
				ErrorMessage("Failed to Initialize WindowProc");
				PostQuitMessage(0);
			}
		}
	}
	else { //�擾���ł��� -> �R�Â����ł��Ă���
		return This->WindowProc(hwnd, uMsg, wParam, lParam);
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam); //���ɂ��Ȃ��Ƃ��̓f�t�H���g�̃E�B���h�E�v���V�[�W����
}