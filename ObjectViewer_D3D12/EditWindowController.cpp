#pragma once
#include"EditWindowController.h"
EditWindowController::EditWindowController(HINSTANCE hInstance, int window_width, int window_height) : BaseWindowController(hInstance, window_width, window_height) {
	for (int i = 0; i < m_editCnt - 1; i++) {
		m_edValue[i] = 0;
	}
	m_edValue[m_editCnt - 7] = 1;
}

EditWindowController::~EditWindowController() {

}

HRESULT EditWindowController::CreateChildWindow() {
	//�G�f�B�b�g�{�b�N�X�傫��
	m_eb_width = m_client_width / 4;
	m_eb_height = 25;
	//�G�f�B�b�g�{�b�N�X�ʒu
	for (int i = 0; i < 3; i++) {
		m_eb_offsetX[i] = 90 + (m_eb_width + 10) * i;
	}
	for (int i = 0; i < 10; i++) {
		m_eb_offsetY[i] = (m_eb_height + 5) * i;
	}

	//�G�f�B�b�g�{�b�N�X �̐���
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 3; j++) {
			if (FAILED(CreateEditBox(m_ehwnd[i * 3 + j], nullptr, m_eb_offsetX[j], m_eb_offsetY[i], m_eb_width, m_eb_height, m_editID + i * 3 + j))) {
				return ErrorMessage("Failed to Create ChildWindow");
			}
		}
	}
	//N��D�̓G�f�B�b�g�{�b�N�X�����
	for (int i = 0; i < 2; i++) {
		if (FAILED(CreateEditBox(m_ehwnd[i + 18], nullptr, m_eb_offsetX[0], m_eb_offsetY[6 + i], m_eb_width, m_eb_height, m_editID + i + 18))) {
			return ErrorMessage("Failed to Create ChildWindow");
		}
	}
	//���C�g�̃J���[�ƌ���
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 3; j++) {
			if (FAILED(CreateEditBox(m_ehwnd[20 + i * 3 + j], nullptr, m_eb_offsetX[j], m_eb_offsetY[8 + i], m_eb_width, m_eb_height, m_editID + 20 + i * 3 + j))) {
				return ErrorMessage("Failed to Create ChildWindow");
			}
		}
	}

	//�G�f�B�b�g�{�b�N�X�ɐ��l��\��
	for (int i = 0; i < m_editCnt; i++) {
		SetWindowText(m_ehwnd[i], std::to_string(m_edValue[i]).c_str());
	}
	return S_OK;
}

void EditWindowController::UpdateEditBoxTransform(XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 siz) {
	m_edValue[0] = pos.x;
	m_edValue[1] = pos.y;
	m_edValue[2] = pos.z;

	m_edValue[3] = rot.x;
	m_edValue[4] = rot.y;
	m_edValue[5] = rot.z;

	m_edValue[6] = siz.x;
	m_edValue[7] = siz.y;
	m_edValue[8] = siz.z;

	//�G�f�B�b�g�{�b�N�X�ɐ��l�𔽉f
	for (int i = 0; i < 9; i++) {
		SetWindowText(m_ehwnd[i], std::to_string(m_edValue[i]).c_str());
	}
}

void EditWindowController::UpdateEditBoxMaterial(XMFLOAT4 amb, XMFLOAT4 dif, XMFLOAT4 spe, float N) {
	m_edValue[9] = amb.x;
	m_edValue[10] = amb.y;
	m_edValue[11] = amb.z;

	m_edValue[12] = dif.x;
	m_edValue[13] = dif.y;
	m_edValue[14] = dif.z;

	m_edValue[15] = spe.x;
	m_edValue[16] = spe.y;
	m_edValue[17] = spe.z;

	m_edValue[18] = N;

	//�G�f�B�b�g�{�b�N�X�ɐ��l�𔽉f
	for (int i = 0; i < 10; i++) {
		SetWindowText(m_ehwnd[i + 9], std::to_string(m_edValue[i + 9]).c_str());
	}
}

LRESULT EditWindowController::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	HDC hdc;
	int id = 0;
	switch (uMsg) {
	case WM_CREATE :

		return 0;
	case WM_PAINT :

		hdc = BeginPaint(hwnd,&ps);
		for (int i = 0; i < 10; i++) {
			TextOut(hdc, 5, m_eb_offsetY[i], m_eb_text[i], strlen(m_eb_text[i]));
		}
		EndPaint(hwnd, &ps);
		
		return 0;
	case WM_MOUSEWHEEL : //�}�E�X�z�C�[��
		if (m_curID != -1 && m_curID != m_editCnt - 7) {
			m_editFlag = true;
			//�}�E�X�z�C�[���̉�]����
			signed short wheelDir = (signed short)HIWORD(wParam) / 120;
			m_edValue[m_curID] += m_edValue[m_editCnt - 7] * wheelDir;
			//������ɖ߂��ăG�f�B�b�g�{�b�N�X�ɃZ�b�g
			SetWindowText(m_ehwnd[m_curID], std::to_string(m_edValue[m_curID]).c_str());
		}
		return 0;
	case WM_COMMAND :
		switch (HIWORD(wParam)) {
		case EN_CHANGE : //�G�f�B�b�g�{�b�N�X�̒��g���ύX����A���ǂ�������̃G�f�B�b�g�{�b�N�X�Ƀt�H�[�J�X�������Ă���Ƃ�
			if (m_curID != -1 && m_isFocusEb) {
				m_editFlag = true;
				//������Ƃ��Ď擾���A�����ɂ��ă����o�ϐ��ɑ��
				LPTSTR dataTxt = (LPTSTR)calloc((GetWindowTextLength(m_ehwnd[m_curID]) + 1), sizeof(char));
				GetWindowText(m_ehwnd[m_curID], dataTxt, GetWindowTextLength(m_ehwnd[m_curID]) + 1);
				try {
					m_edValue[m_curID] = std::stof(dataTxt);
				}
				catch (std::exception& e) { //�G�f�B�b�g�{�b�N�X�ɐ����ȊO����͂��Ă��Ƃ�
					//�������Ȃ�
				}
			}
			break;
		case EN_SETFOCUS:

			id = LOWORD(wParam);
			if (id >= m_editID && id <= m_editID + m_editCnt - 1) {
				m_isFocusEb = true;
				m_curID = id;
			}
			break;
		case EN_KILLFOCUS:
			m_isFocusEb = false;
			break;
		}
		return 0;
	case WM_QUIT:
			PostQuitMessage(0);
			return 0;
	}
	
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}