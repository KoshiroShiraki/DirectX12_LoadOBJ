#pragma once
#include"DX12Object3D.h"

DX12Object3D::DX12Object3D() {
	m_vertices.resize(4);
	m_vertices[0].position = XMFLOAT3(-100, 0, 100);
	m_vertices[1].position = XMFLOAT3(100, 0, 100);
	m_vertices[2].position = XMFLOAT3(100, 0, -100);
	m_vertices[3].position = XMFLOAT3(-100, 0, -100);
	m_vertices[0].normal = XMFLOAT3(0, 0, -1);
	m_vertices[1].normal = XMFLOAT3(0, 0, -1);
	m_vertices[2].normal = XMFLOAT3(0, 0, -1);
	m_vertices[3].normal = XMFLOAT3(0, 0, -1);
	m_vertices[0].uv = XMFLOAT2(0, 0);
	m_vertices[1].uv = XMFLOAT2(1, 0);
	m_vertices[2].uv = XMFLOAT2(1, 1);
	m_vertices[3].uv = XMFLOAT2(0, 1);

	m_indices.resize(6);
	m_indices[0] = 0;
	m_indices[1] = 1;
	m_indices[2] = 2;
	m_indices[3] = 0;
	m_indices[4] = 2;
	m_indices[5] = 3;

	m_material.ambient = XMFLOAT4(0, 0, 0, 1);
	m_material.diffuse = XMFLOAT4(1, 1, 1, 1);
	m_material.specular = XMFLOAT4(1, 1, 1, 1);
	m_material.N = 0;
}

DX12Object3D::~DX12Object3D() {
	Release();
}

HRESULT DX12Object3D::CreateVertexBuffer(ID3D12Device* device) {
	HRESULT hr;

	//ヒープ設定
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	
	//リソース設定
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = m_vertices.size() * sizeof(DX12Vertex);
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count = 1;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	
	//頂点バッファの生成
	hr = device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_vertexBuffer));
	if (FAILED(hr)) {
		return ErrorMessage("Failed to Create VertexBuffer");
	}
	
	//頂点データのマップ
	DX12Vertex* vertMap = nullptr;
	hr = m_vertexBuffer->Map(0, nullptr, (void**)&vertMap);
	if (FAILED(hr)) {
		return ErrorMessage("Failed to Map VertexBuffer");
	}
	std::copy(std::begin(m_vertices), std::end(m_vertices), vertMap);
	m_vertexBuffer->Unmap(0, nullptr);

	//頂点バッファビューの設定
	m_vbv = {};
	m_vbv.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vbv.SizeInBytes = m_vertexBuffer->GetDesc().Width;
	m_vbv.StrideInBytes = sizeof(DX12Vertex);
	return S_OK;
}

HRESULT DX12Object3D::CreateIndexBuffer(ID3D12Device* device) {
	HRESULT hr;

	//ヒープ設定
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	//リソース設定
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = m_indices.size() * sizeof(unsigned);
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count = 1;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	
	//インデックスバッファの生成
	hr = device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_indexBuffer));
	if (FAILED(hr)) {
		return ErrorMessage("Failed to Create IndexBuffer");
	}

	//インデックスデータのマップ
	unsigned* idxMap = nullptr;
	hr = m_indexBuffer->Map(0, nullptr, (void**)&idxMap);
	if (FAILED(hr)) {
		return ErrorMessage("Failed to Map IndexBuffer");
	}
	std::copy(std::begin(m_indices), std::end(m_indices), idxMap);
	m_indexBuffer->Unmap(0, nullptr);

	//インデックスバッファビューの設定
	m_ibv = {};
	m_ibv.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_ibv.Format = DXGI_FORMAT_R32_UINT;
	m_ibv.SizeInBytes = m_indices.size() * sizeof(unsigned);
}

HRESULT DX12Object3D::CreateMaterialBuffer(ID3D12Device* device) {
	HRESULT hr;

	//ヒープ設定
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	//リソース設定
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = sizeof(DX12Material);
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count = 1;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	
	//マテリアルバッファの生成
	hr = device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_materialBuffer));
	if (FAILED(hr)) {
		return ErrorMessage("Failed to Create MaterialBuffer");
	}

	//マテリアルデータのマップ
	DX12Material* matMap;
	hr = m_materialBuffer->Map(0, nullptr, (void**)&matMap);
	if (FAILED(hr)) {
		return ErrorMessage("Failed to Map MaterialBuffer");
	}
	matMap->ambient = m_material.ambient;
	matMap->diffuse = m_material.diffuse;
	matMap->specular = m_material.specular;
	matMap->N = m_material.N;
	m_materialBuffer->Unmap(0, nullptr);

	//ディスクリプタヒープの生成
	D3D12_DESCRIPTOR_HEAP_DESC matHeapDesc;
	matHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	matHeapDesc.NodeMask = 0;
	matHeapDesc.NumDescriptors = 1;
	matHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	hr = device->CreateDescriptorHeap(&matHeapDesc, IID_PPV_ARGS(&m_matDescHeap));
	if (FAILED(hr)) {
		return ErrorMessage("Failed to Create MaterialDescriptorHeap");
	}

	//マテリアルバッファビューの生成
	D3D12_CONSTANT_BUFFER_VIEW_DESC matCBVDesc = { };
	matCBVDesc.BufferLocation = m_materialBuffer->GetGPUVirtualAddress();
	matCBVDesc.SizeInBytes = sizeof(DX12Material);
	device->CreateConstantBufferView(&matCBVDesc, m_matDescHeap->GetCPUDescriptorHandleForHeapStart());
}

HRESULT DX12Object3D::CreateTextureBuffer(ID3D12Device* device, std::wstring texFilePath) {
	HRESULT hr;
	//テクスチャを再割り当てする際に、オブジェクトをすべて開放する
	if (m_textureBuffer != nullptr) {
		m_textureBuffer->Release();
		m_textureBuffer = nullptr;

		m_texDescHeap->Release();
		m_texDescHeap = nullptr;
	}

	//テクスチャロード
	ScratchImage sImg = {};
	TexMetadata md = {};
	//LoadTexture(texFilePath, sImg, md);

	//ヒープ設定
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	//リソース設定
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = m_vertices.size() * sizeof(DX12Vertex);
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count = 1;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//ディスクリプタヒープの生成
	D3D12_DESCRIPTOR_HEAP_DESC texHeapDesc = {};
	texHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	texHeapDesc.NodeMask = 0;
	texHeapDesc.NumDescriptors = 1;
	texHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	hr = device->CreateDescriptorHeap(&texHeapDesc, IID_PPV_ARGS(&m_texDescHeap));
	if (FAILED(hr)) {
		return ErrorMessage("Failed to Create TextureDescriptorHeap");
	}


	/*texBuffers.resize(materials.size());
	for (int i = 0; i < materials.size(); i++) {
		wchar_t path[MAX_PATH_LENGTH];
		mbstowcs(path, materials[i].ambTexPath, MAX_PATH_LENGTH);
		hr = OBJ_CreateTextureBuffer(path, MAX_PATH_LENGTH, i, 0, device);
		mbstowcs(path, materials[i].difTexPath, MAX_PATH_LENGTH);
		//std::cout << materials[i].difTexPath << std::endl;
		hr = OBJ_CreateTextureBuffer(path, MAX_PATH_LENGTH, i, 1, device);
		mbstowcs(path, materials[i].speTexPath, MAX_PATH_LENGTH);
		hr = OBJ_CreateTextureBuffer(path, MAX_PATH_LENGTH, i, 2, device);
		//std::cout << materials[i].speTexPath << std::endl;

		//Create shaderResourceView
		/*
		ShaderResourceView must be Created whether Texture load completed or not
		*/
		/*D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		D3D12_CPU_DESCRIPTOR_HANDLE heapH = textureDescHeap->GetCPUDescriptorHandleForHeapStart();
		heapH.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * (3 * i); //StartPosition
		//Create 3 Descriptor
		device->CreateShaderResourceView(texBuffers[i].ambTexBuffer, &srvDesc, heapH);
		heapH.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		device->CreateShaderResourceView(texBuffers[i].difTexBuffer, &srvDesc, heapH);
		heapH.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		device->CreateShaderResourceView(texBuffers[i].speTexBuffer, &srvDesc, heapH);
	}*/

	return S_OK;
}

HRESULT DX12Object3D::Create(ID3D12Device* device) {
	if (FAILED(CreateVertexBuffer(device))) {
		return E_FAIL;
	}
	if (FAILED(CreateIndexBuffer(device))) {
		return E_FAIL;
	}
	if (FAILED(CreateMaterialBuffer(device))) {
		return E_FAIL;
	}

	return S_OK;
}

HRESULT DX12Object3D::Draw(ID3D12GraphicsCommandList* cmdList, ID3D12DescriptorHeap* cbvHeap, UINT cbOffset) {
	//定数バッファをセット
	cmdList->SetDescriptorHeaps(1, &cbvHeap);
	D3D12_GPU_DESCRIPTOR_HANDLE handle = cbvHeap->GetGPUDescriptorHandleForHeapStart();
	handle.ptr += cbOffset;
	cmdList->SetGraphicsRootDescriptorTable(0, handle);

	//頂点とインデックスをセット
	cmdList->IASetVertexBuffers(0, 1, &m_vbv);
	cmdList->IASetIndexBuffer(&m_ibv);

	//テクスチャをセット
	//cmdList->SetDescriptorHeaps(1, &m_texDescHeap);
	//cmdList->SetGraphicsRootDescriptorTable(2, m_texDescHeap->GetGPUDescriptorHandleForHeapStart());
	
	//マテリアルをセット
	cmdList->SetDescriptorHeaps(1, &m_matDescHeap);
	cmdList->SetGraphicsRootDescriptorTable(1, m_matDescHeap->GetGPUDescriptorHandleForHeapStart());
	
	//描画
	cmdList->DrawIndexedInstanced(m_indices.size(), 1, 0, 0, 0);

	return S_OK;
}

void DX12Object3D::UpdateMaterial(float* data) {
	m_material.ambient = XMFLOAT4(data[9], data[10], data[11], 1.0f);
	m_material.diffuse = XMFLOAT4(data[12], data[13], data[14], 1.0f);
	m_material.specular = XMFLOAT4(data[15], data[16], data[17], 1.0f);
	m_material.N = data[18];

	//マテリアルデータのマップ
	DX12Material* matMap;
	HRESULT hr = m_materialBuffer->Map(0, nullptr, (void**)&matMap);
	if (FAILED(hr)) {
		ErrorMessage("Failed to Map MaterialBuffer");
	}
	matMap->ambient = m_material.ambient;
	matMap->diffuse = m_material.diffuse;
	matMap->specular = m_material.specular;
	matMap->N = m_material.N;
	m_materialBuffer->Unmap(0, nullptr);
}

template<class T>
void ReleaseVector(std::vector<T>& vector) {
	vector.clear();
	vector.shrink_to_fit();
}

void DX12Object3D::Release() {
	if (m_vertexBuffer) m_vertexBuffer->Release();
	if (m_indexBuffer) m_indexBuffer->Release();
	if (m_materialBuffer) m_materialBuffer->Release();
	if (m_textureBuffer) m_textureBuffer->Release();

	ReleaseVector(m_vertices);
	ReleaseVector(m_indices);
}