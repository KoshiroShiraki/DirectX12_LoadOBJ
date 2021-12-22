#pragma once
#include"ErrorDebug.h"

HRESULT  ErrorMessage(std::string errMsg) {
	MessageBox(nullptr, errMsg.c_str(), nullptr, MB_OK);
	return E_FAIL;
}