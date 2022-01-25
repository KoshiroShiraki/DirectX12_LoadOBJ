#include"RenameWindowController.h"

RenameWindowController::RenameWindowController(HINSTANCE hInstance, int window_width, int window_height) : BaseWindowController(hInstance, window_width, window_height) {
	m_eb_width = window_width;
}

RenameWindowController::~RenameWindowController() {

}

HRESULT RenameWindowController::CreateChildWindow() {
	//エディットボックスの生成
	if (FAILED(CreateEditBox(m_ehwnd, nullptr, 5, 0, m_eb_width - 15, m_eb_height, 0))) {
		return ErrorMessage("Failed to CreateEditBox");
	}
	//ボタンの生成
	if (FAILED(CreateButton(m_bhwnd, "OK", m_window_width / 2 - m_btn_width / 2, m_eb_height + 5, m_btn_width, m_btn_height, m_okbtnID))) {
		return ErrorMessage("Failed to CreateButton");
	}
	return S_OK;
}

LRESULT RenameWindowController::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_CREATE:

		return 0;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case m_okbtnID:
			std::cout << "押された" << std::endl;
			m_renameCompleted = true; //リネーム完了のフラグを立てる
			//エディットボックスの文字列を取得
			name = (LPTSTR)calloc((GetWindowTextLength(m_ehwnd) + 1), sizeof(char));
			GetWindowText(m_ehwnd, name, GetWindowTextLength(m_ehwnd) + 1);
			
			break;
		}

		return 0;
	}


	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}