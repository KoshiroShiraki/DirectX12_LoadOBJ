#pragma once
#include<Windows.h>
#include<d3d12.h>
#include<d3dx12.h>
#include<d3dcompiler.h>
#include<iostream>
#include"ErrorDebug.h"

class DX12Shader {
public :
	DX12Shader();
	~DX12Shader();

private :
	ID3DBlob* m_buffer = nullptr;

public :
	HRESULT CreateShader(std::wstring filename, std::string entryPoint, std::string shaderType, ID3DBlob* errorBlob);
	LPVOID GetBufferPointer();
	SIZE_T GetBufferSize();
};
