#pragma once
#include"DX12Shader.h"

DX12Shader::DX12Shader() {

}

DX12Shader::~DX12Shader() {
	if (m_buffer)m_buffer->Release();
}

HRESULT DX12Shader::CreateShader(std::wstring filename, std::string entryPoint, std::string shaderType, ID3DBlob* errorBlob) {
	HRESULT hr;

	std::wstring ex_hlsl = L".hlsl";
	std::wstring ex_cso = L".cso";

	hr = D3DCompileFromFile((filename + ex_hlsl).c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), shaderType.c_str(), D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &m_buffer, &errorBlob);
	if (FAILED(hr)) {
		if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			hr = D3DReadFileToBlob((filename + ex_cso).c_str(), &m_buffer);
			if (FAILED(hr)) {
				std::cout << "Error : There is no " << filename.c_str() << std::endl;
				return hr;
			}
		}
		else {
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";

			std::cout << "Failed to Compile " << filename.c_str() << std::endl;
			return hr;
		}
	}

	return S_OK;
}

LPVOID DX12Shader::GetBufferPointer() {
	return m_buffer->GetBufferPointer();
}

SIZE_T DX12Shader::GetBufferSize() {
	return m_buffer->GetBufferSize();
}