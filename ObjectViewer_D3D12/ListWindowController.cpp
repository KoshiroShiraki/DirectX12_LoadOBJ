#pragma once
#include"ListWindowController.h"

ListWindowController::ListWindowController(HINSTANCE hInstance, int window_width, int window_height) : BaseWindowController(hInstance, window_width, window_height) {

}

ListWindowController::~ListWindowController() {

}

HRESULT ListWindowController::CreateChildWindow() {
	//newボタン大きさ
	int btn_width = m_client_width / 5.5;
	int btn_height = m_client_height / 40;
	//ボタン位置
	int btn_offsetX = 0;
	int btn_offsetY[m_btnCnt] = { 0,btn_height * 2,btn_height * 3,btn_height * 4,btn_height * 5,btn_height * 6,btn_height * 7 };

	//コンボボックス大きさ
	int cb_width = m_client_width - btn_width;
	int cb_height = m_client_height;
	//コンボボックス位置
	int cb_offsetX = btn_width;
	int cb_offsetY = 0;


	//リストボックス大きさ
	int lb_width = (m_client_width - btn_width) / 2 - 30;
	int lb_height = m_client_height - btn_height * 2 - 15;
	//リストボックス位置
	int lb_offsetX[m_lbCnt] = { btn_width + 15, lb_offsetX[0] + lb_width + 15};
	int lb_offsetY[m_lbCnt] = { btn_height * 2, btn_height * 2 };

	if (FAILED(CreateButton(m_bhwnd[0], m_btnName[0], btn_offsetX, btn_offsetY[0], btn_width, btn_height, m_newbtnID))) {
		return ErrorMessage("Failed to Create ChildWindow");
	}
	if (FAILED(CreateButton(m_bhwnd[1], m_btnName[1], btn_offsetX, btn_offsetY[1], btn_width, btn_height, m_renamebtnID))) {
		return ErrorMessage("Failed to Create ChildWindow");
	}
	if (FAILED(CreateButton(m_bhwnd[2], m_btnName[2], btn_offsetX, btn_offsetY[2], btn_width, btn_height, m_duplicatebtnID))) {
		return ErrorMessage("Failed to Create ChildWindow");
	}
	if (FAILED(CreateButton(m_bhwnd[3], m_btnName[3], btn_offsetX, btn_offsetY[3], btn_width, btn_height, m_savebtnID))) {
		return ErrorMessage("Failed to Create ChildWindow");
	}
	if (FAILED(CreateButton(m_bhwnd[4], m_btnName[4], btn_offsetX, btn_offsetY[4], btn_width, btn_height, m_deletebtnID))) {
		return ErrorMessage("Failed to Create ChildWindow");
	}
	if (FAILED(CreateButton(m_bhwnd[5], m_btnName[5], btn_offsetX, btn_offsetY[5], btn_width, btn_height, m_savescenebtnID))) {
		return ErrorMessage("Failed to Create ChildWindow");
	}
	if (FAILED(CreateButton(m_bhwnd[6], m_btnName[6], btn_offsetX, btn_offsetY[6], btn_width, btn_height, m_quitbtnID))) {
		return ErrorMessage("Failed to Create ChildWindow");
	}

	if (FAILED(CreateDropDownComboBox(m_chwnd, "ComboBox", cb_offsetX, cb_offsetY, cb_width, cb_height,m_cbID))) {
		return ErrorMessage("Failed to Create ChildWindow");
	}
	if(FAILED(CreateListBox(m_lhwnd[0], "ListBox", lb_offsetX[0], lb_offsetY[0], lb_width, lb_height, m_lbParentID))){
		return ErrorMessage("Failed to Create ChildWindow");
	}
	if (FAILED(CreateListBox(m_lhwnd[1], "ListBox", lb_offsetX[1], lb_offsetY[1], lb_width, lb_height, m_lbChildID))) {
		return ErrorMessage("Failed to Create ChildWindow");
	}
	return S_OK;
}

HRESULT ListWindowController::InitChildWindow() {

	return S_OK;
}

LRESULT ListWindowController::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) { //メッセージ内容を調べる
	case WM_COMMAND:
		switch (LOWORD(wParam)) { //IDを調べる
		case m_newbtnID : //"new"ボタンの時
			if (!m_isLoad) {
				m_isLoad = true;
			}
			return 0;
		case m_deletebtnID : //"delete"ボタンの時
			if (!m_isDelete) {
				m_isDelete = true;
			}
			return 0;
		case m_duplicatebtnID : //"duplicate"ボタンの時
			if (!m_isDuplicate) {
				m_isDuplicate = true;
			}
			return 0;
		case m_renamebtnID: //"Rename"ボタンの時
			if (!m_isRename && m_parentIdx != -1) {
				m_isRename = true;
			}
			return 0;
		case m_savebtnID: //"save"ボタンの時
			if (!m_isSave) {
				m_isSave = true;
			}
			return 0;
		case m_quitbtnID : //"quit"ボタンの時
			PostQuitMessage(0);
			return 0;
		case m_cbID : //"ComboBox"の時
			switch (HIWORD(wParam)) { //メッセージ内容を調べる
			case CBN_SELCHANGE :
				m_loadIdx = SendMessage(m_chwnd, CB_GETCURSEL, 0, 0); //選択項目のインデックスを取得
				return 0;
			}
			return 0;
		case m_lbParentID : //"ParentListBox"の時
			switch (HIWORD(wParam)) { //メッセージ内容を調べる
			case LBN_SELCHANGE :
				int p = m_parentIdx;
				m_parentIdx = SendMessage(m_lhwnd[0], LB_GETCURSEL, 0, 0);
				m_childIdx = 0;
				if (p != m_parentIdx) {
					m_isParentChanged = true;
					m_isChildChanged = true;
				}
				return 0;
			}
			return 0;
		case m_lbChildID: //"ChildListBox"の時
			switch (HIWORD(wParam)) { //メッセージ内容を調べる
			case LBN_SELCHANGE :
				int p = m_childIdx;
				m_childIdx = SendMessage(m_lhwnd[1], LB_GETCURSEL, 0, 0);
				if (p != m_childIdx) {
					m_isChildChanged = true;
				}
				return 0;
			}
			return 0;
		}

		return 0;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}