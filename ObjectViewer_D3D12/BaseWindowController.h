#pragma once
#include<Windows.h>
#include<iostream>
#include<vector>
#include<string>
#include"ErrorDebug.h"
#include"PathController.h"

class BaseWindowController {
public:
	HWND m_hwnd; //�E�B���h�E�n���h��
	WNDCLASSEX m_wcx; //�E�B���h�E�N���X��`

protected:
	ATOM m_atom; //�E�B���h�E�o�^�`�F�b�N�p
	HINSTANCE m_hinst; //�A�v���P�[�V�����C���X�^���X�n���h��

public:
	int m_window_width; //�E�B���h�E��
	int m_window_height; //�E�B���h�E����

	int m_client_width; //�N���C�A���g�̈敝
	int m_client_height; //�N���C�A���g�̈捂��

public:
	BaseWindowController(HINSTANCE hInstance, int window_width, int window_height);
	~BaseWindowController();

public:
	HRESULT InitWindow(LPCSTR className, LPCSTR windowName); //�E�B���h�E������(�E�B���h�E�N���X�o�^ -> ���C���E�B���h�E���� -> �q�E�B���h�E����)
	virtual HRESULT CreateChildWindow() = 0;

protected:
	HRESULT CreateMainWindow(LPCSTR className, LPCSTR windowName); //�p���N���X�͂��ꂼ��K��m_hwnd�n���h���ɕR�Â�����̃��C���E�B���h�E������
	HRESULT CreateButton(HWND& hBtn, LPCSTR text, int offsetX, int offsetY, int width, int height, int btnID); //�{�^���𐶐�����
	HRESULT CreateDropDownComboBox(HWND& hCb, LPCSTR text, int offsetX, int offsetY, int width, int height, int cbID); //�R���{�{�b�N�X�𐶐�����
	HRESULT CreateListBox(HWND& hLb, LPCSTR text, int offsetX, int offsetY, int width, int height, int lbID); //���X�g�{�b�N�X�𐶐�����
	HRESULT CreateEditBox(HWND& hEb, LPCSTR text, int offsetX, int offsetY, int width, int height, int ebID);
	HRESULT CreateSlider(HWND& hSb, LPCSTR text, int offsetX, int offsetY, int width, int height, int sID);
	void DeleteWindow(); //�E�B���h�E�̍폜�ƃN���X�̓o�^����

public:
	virtual LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0; //�p���N���X���Ƃ̎��������ۂɋL�q����E�B���h�E�v���V�[�W��
	static LRESULT CALLBACK baseWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam); //�E�B���h�E�N���X�ɐݒ肷��p�̃E�B���h�E�v���V�[�W��
};