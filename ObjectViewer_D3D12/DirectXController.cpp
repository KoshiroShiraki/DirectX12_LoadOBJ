#pragma once
#include"DirectXController.h"

DirectXController::DirectXController() {
	//ビューポートの設定
	m_vp.Width = window_Width;
	m_vp.Height = window_Height;
	m_vp.TopLeftX = 0;
	m_vp.TopLeftY = 0;
	m_vp.MaxDepth = 1.0f;
	m_vp.MinDepth = 0.0f;

	//シザー矩形の設定
	m_sr.top = 0;
	m_sr.left = 0;
	m_sr.right = m_sr.left + window_Width;
	m_sr.bottom = m_sr.top + window_Height;
}

DirectXController::~DirectXController() {

}

HRESULT DirectXController::InitD3D(HWND hwnd) {
	HRESULT hr;

#ifdef _DEBUG
	EnableDebugLayer();
#endif

	/*-----ファクトリの生成-----*/
	hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&m_factory));
	if (FAILED(hr)) {
		std::cout << "Failed to Create m_factory\n";
		return hr;
	}

	/*-----デバイスの生成-----*/
	//enumerate Video Adapter
	std::vector <IDXGIAdapter*> adapters;
	IDXGIAdapter* tmpAdapter = nullptr;
	for (int i = 0; m_factory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; i++) {
		adapters.push_back(tmpAdapter);
	}
	//アダプタの設定
	for (auto adpt : adapters) {
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);
		std::wstring strDesc = adesc.Description;
		if (strDesc.find(L"NVIDIA") != std::string::npos) {
			tmpAdapter = adpt;
			std::wcout << "Adaptor : " << strDesc << "\n";
			break;
		}
	}
	//機能レベルの列挙
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};
	//デバイスの生成
	D3D_FEATURE_LEVEL featureLevel;
	for (auto lv : levels) {
		hr = D3D12CreateDevice(tmpAdapter, lv, IID_PPV_ARGS(&m_device));
		if (SUCCEEDED(hr)) {
			featureLevel = lv;
			std::cout << lv << std::endl;
			break;
		}
	}
	if (FAILED(hr)) {
		std::cout << "Failed to Create m_device\n";
		return hr;
	}

	/*-----コマンドアロケータの生成-----*/
	hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_cmdAllocator));
	if (FAILED(hr)) {
		std::cout << "Falied to Create m_CommandAllocator\n";
		return hr;
	}

	/*-----コマンドリストの生成-----*/
	hr = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAllocator, nullptr, IID_PPV_ARGS(&m_cmdList));
	if (FAILED(hr)) {
		std::cout << "Failed to Create m_CommandList\n";
		return hr;
	}

	/*-----コマンドキューの生成-----*/
	//コマンドキューの設定
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	//コマンドキューの生成
	hr = m_device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&m_cmdQueue));
	if (FAILED(hr)) {
		std::cout << "Failed to Create m_CommandQueue\n";
		return hr;
	}

	/*-----スワップチェインの生成-----*/
	//スワップチェインの設定
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.Width = window_Width;
	swapchainDesc.Height = window_Height;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = RT_BUFFER_COUNT;
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	//スワップチェインの生成
	hr = m_factory->CreateSwapChainForHwnd(m_cmdQueue, hwnd, &swapchainDesc, nullptr, nullptr, (IDXGISwapChain1**)&m_swapchain);
	if (FAILED(hr)) {
		std::cout << "Failed to Create m_SwapChain\n";
		return hr;
	}

	/*-----フェンスの生成-----*/
	hr = m_device->CreateFence(m_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
	if (FAILED(hr)) {
		std::cout << "Failed to Create m_Fence\n";
		return hr;
	}

	return S_OK;
}

HRESULT DirectXController::CreateBackBuffers() {
	HRESULT hr;

	/*-----バックバッファの生成-----*/
	//ディスクリプタヒープの設定
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = RT_BUFFER_COUNT;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	//ディスクリプタヒープの生成
	hr = m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_frtvHeap));
	if (FAILED(hr)) {
		std::cout << "Failed to Create Descriptor Heap\n";
		return 0;
	}
	//ディスクリプタ(レンダーターゲットビュー)の生成
	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_frtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (int i = 0; i < RT_BUFFER_COUNT; i++) {
		hr = m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_backBuffers[i]));
		m_device->CreateRenderTargetView(m_backBuffers[i], nullptr, handle);
		handle.ptr += m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	return S_OK;
}

HRESULT DirectXController::CreateRenderBuffers() {
	HRESULT hr;

	/*----RenderBufferの生成-----*/
	if (m_backBuffers[0] == nullptr) {
		std::cout << "BackBuffers are not Created" << std::endl;
		return E_FAIL;
	}
	//リソース設定をバックバッファから使いまわす
	D3D12_RESOURCE_DESC resDesc = m_backBuffers[0]->GetDesc();
	//ヒーププロパティの設定
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	//クリアカラーの設定
	D3D12_CLEAR_VALUE cv;
	float clearColor[4] = { 0.5f,0.5f,0.7f,1.0f };
	std::copy(std::begin(clearColor), std::end(clearColor), cv.Color);
	cv.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//あらかじめ決めておいた数だけRenderBufferを生成する
	for (int i = 0; i < RT_MULTIPASS_COUNT; i++) {
		hr = m_device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &cv, IID_PPV_ARGS(&m_mrenderBuffers[i]));
		if (FAILED(hr)) {
			std::cout << "Failed to Creatae renderImages" << std::endl;
			return hr;
		}
	}

	/*-----レンダーターゲットビューの生成-----*/
	//ディスクリプタヒープの設定
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = RT_BUFFER_COUNT;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	heapDesc.NumDescriptors = RT_MULTIPASS_COUNT;
	//ディスクリプタヒープの生成
	hr = m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_mrtvHeap));
	if (FAILED(hr)) {
		std::cout << "Failed to Create DescriptorHeap" << std::endl;
		return hr;
	}
	//レンダーターゲットビューの生成
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_mrtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (int i = 0; i < RT_MULTIPASS_COUNT; i++) {
		m_device->CreateRenderTargetView(m_mrenderBuffers[i], &rtvDesc, handle);
		handle.ptr += m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	/*-----シェーダリソースビューの生成-----*/
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	hr = m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_msrvHeap));
	//ディスクリプタヒープの生成
	if (FAILED(hr)) {
		std::cout << "Failed to Create DescriptorHeap" << std::endl;
		return hr;
	}
	//シェーダリソースビューの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = rtvDesc.Format;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//シェーダリソースビューの生成
	handle = m_msrvHeap->GetCPUDescriptorHandleForHeapStart();
	for (int i = 0; i < RT_MULTIPASS_COUNT; i++) {
		m_device->CreateShaderResourceView(m_mrenderBuffers[i], &srvDesc, handle);
		handle.ptr += m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
}

HRESULT DirectXController::CreateDepthStencilBuffer() {
	HRESULT hr;

	/*-----Depth/StencilBufferの生成-----*/
	//深度/ステンシルバッファの定義
	D3D12_RESOURCE_DESC depthResDesc = {};
	depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResDesc.Width = window_Width;
	depthResDesc.Height = window_Height;
	depthResDesc.DepthOrArraySize = 1;
	depthResDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthResDesc.SampleDesc.Count = 1;
	depthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	//ヒープの設定
	D3D12_HEAP_PROPERTIES depthHeapProp = {};
	depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	depthHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	depthHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	//深度/ステンシルバッファのクリア値
	D3D12_CLEAR_VALUE depthClearValue = {};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	//深度/ステンシルバッファの生成
	hr = m_device->CreateCommittedResource(&depthHeapProp, D3D12_HEAP_FLAG_NONE, &depthResDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue, IID_PPV_ARGS(&m_mdepthBuffer));
	if (FAILED(hr)) {
		std::cout << "Failed to Create m_DepthStencilBuffer" << std::endl;;
		return hr;
	}

	/*-----Depth/StencilViewの生成-----*/
	//ディスクリプタヒープの設定
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hr = m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_mdsvHeap));
	if (FAILED(hr)) {
		std::cout << "Failed to Create m_mdsvHeap" << std::endl;;
		return hr;
	}
	//ディスクリプタ(深度/ステンシルビューの生成)
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	m_device->CreateDepthStencilView(m_mdepthBuffer, &dsvDesc, m_mdsvHeap->GetCPUDescriptorHandleForHeapStart());
}

HRESULT DirectXController::CreateShadowBuffer() {
	HRESULT hr;

	//リソースは深度バッファとして生成する
	//リソース設定
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Width = window_Width;
	resDesc.Height = window_Height;
	resDesc.DepthOrArraySize = 1;
	resDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	resDesc.SampleDesc.Count = 1;
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	//ヒープの設定
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	//深度/ステンシルバッファのクリア値
	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	//シャドウバッファの生成
	hr = m_device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearValue, IID_PPV_ARGS(&m_shadowBuffer));
	if (FAILED(hr)) {
		return ErrorMessage("Failed to Create ShadowBuffer");
	}

	//深度バッファとして使うためのデプス/ステンシルビューを生成する
	//ディスクリプタヒープの設定
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hr = m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_sdsvHeap));
	if (FAILED(hr)) {
		std::cout << "Failed to Create m_sdsvHeap" << std::endl;;
		return hr;
	}
	//ディスクリプタ(深度/ステンシルビューの生成)
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	m_device->CreateDepthStencilView(m_shadowBuffer, &dsvDesc, m_sdsvHeap->GetCPUDescriptorHandleForHeapStart());

	//テクスチャとして使うためのシェーダリソースビューを生成する
	//深度/ステンシルビューに使ったのを書き換える
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	hr = m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_ssrvHeap));
	//ディスクリプタヒープの生成
	if (FAILED(hr)) {
		std::cout << "Failed to Create DescriptorHeap" << std::endl;
		return hr;
	}
	//シェーダリソースビューの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//シェーダリソースビューの生成
	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_ssrvHeap->GetCPUDescriptorHandleForHeapStart();
	m_device->CreateShaderResourceView(m_shadowBuffer, &srvDesc, handle);
	
}

HRESULT DirectXController::CreateRenderResources() {

	if (FAILED(CreateBackBuffers())) {
		return ErrorMessage("Failed to Create BackBuffers");
	}

	if (FAILED(CreateRenderBuffers())) {
		return ErrorMessage("Failed to Create RenderBuffers");
	}

	if (FAILED(CreateDepthStencilBuffer())) {
		return ErrorMessage("Failed to Create DepthStencilBuffer");
	}

	if (FAILED(CreateShadowBuffer())) {
		return ErrorMessage("Failed to Create ShadowBuffer");
	}

	if (FAILED(CreateFinalRenderPolygon())) {
		return ErrorMessage("Failed to Create FinalRenderPolygon");
	}


	return S_OK;
}

HRESULT DirectXController::CreateFinalRenderPolygon() {
	HRESULT hr;
	/*
	最終出力結果テクスチャを貼り付けるポリゴン
	座標変換もすべて必要ない、ビューポートの端から端を覆うUV付きポリゴンを作ればよい
	*/
	SimpleVertex frRect[] = {
		{ {-1, 1, 0.1f}, {0, 0} },
		{ {1, 1, 0.1f},{1, 0} },
		{ {-1, -1, 0.1f}, {0 ,1} },
		{ {1, -1, 0.1f}, {1, 1} }
	};
	/*-----頂点バッファの生成-----*/
	//リソース設定
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = sizeof(frRect);
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count = 1;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//ヒープ設定
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	//頂点バッファの生成
	hr = m_device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_frenderPolygon));
	if (FAILED(hr)) {
		std::cout << "Failed to Create FinalRenderPolygon" << std::endl;
		return hr;
	}

	/*-----頂点データのマップ-----*/
	SimpleVertex* mapData = nullptr;
	m_frenderPolygon->Map(0, nullptr, (void**)(&mapData));
	std::copy(std::begin(frRect), std::end(frRect), mapData);
	m_frenderPolygon->Unmap(0, nullptr);

	/*-----頂点バッファビューの生成-----*/
	m_frenderVbv.BufferLocation = m_frenderPolygon->GetGPUVirtualAddress();
	m_frenderVbv.SizeInBytes = sizeof(frRect);
	m_frenderVbv.StrideInBytes = sizeof(SimpleVertex);

	return S_OK;
}

HRESULT CreateFloorPolygon() {

	return S_OK;
}

HRESULT DirectXController::CreateConstBuffers(Camera& camera, Light& light) {
	HRESULT hr;

	/*-----constBufferの生成-----*/
	//ビュー行列(カメラ視点行列)の生成
	XMFLOAT3 eye(0, 0, -10);
	XMFLOAT3 target(0, 0, eye.z + 1);
	XMFLOAT3 up(0, 1, 0);
	camera.InitCamera(eye, target, up);
	//パースペクティブ行列の生成
	XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(XM_PIDIV2, static_cast<float>(window_Width) / static_cast<float>(window_Height), 1.0f, 10000.0f);
	
	//ビュー行列(ライト視点)の生成
	eye = XMFLOAT3(0, 1000, 0);
	target = XMFLOAT3(0, 0, 5);
	up = XMFLOAT3(0, 1, 0);
	light.InitLight(eye, target, up, XMFLOAT3(0, 0, 0));
	//平衡投影行列の生成
	XMMATRIX shadowProjectionMatrix = XMMatrixOrthographicLH(1000, 1000, 1.0f, 10000.0f);

	//リソース設定
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = sizeof(MatrixData); //カメラ用とライト用の2つ
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count = 1;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//ヒープ設定
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	//定数バッファの生成(通常用)
	hr = m_device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_mconstBuffer)
	);
	if (FAILED(hr)) {
		std::cout << "Failed to Create m_mconstBuffer" << std::endl;
		return hr;
	}
	//定数バッファの生成(シャドウマップ用)
	resDesc.Width = sizeof(ShadowMatrixData);
	hr = m_device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_sconstBuffer)
	);
	if (FAILED(hr)) {
		std::cout << "Failed to Create m_mconstBuffer" << std::endl;
		return hr;
	}
	hr = m_mconstBuffer->Map(0, nullptr, (void**)&m_mmapMatrix);
	if (FAILED(hr)) {
		std::cout << "Failed to Map constBuffer\n";
		return hr;
	}
	(m_mmapMatrix)->v_light = light.m_LightViewMatrix;
	(m_mmapMatrix)->v_camera = camera.viewMatrix;
	(m_mmapMatrix)->p_perspective = projectionMatrix;
	(m_mmapMatrix)->p_orthographic = shadowProjectionMatrix;
	(m_mmapMatrix)->eye = eye;

	hr = m_sconstBuffer->Map(0, nullptr, (void**)&m_smapMatrix);
	(m_smapMatrix)->v_light = light.m_LightViewMatrix;
	(m_smapMatrix)->p = shadowProjectionMatrix;

	/*-----constBufferViewの生成-----*/
	//ディスクリプタヒープの設定
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NodeMask = 0;
	descHeapDesc.NumDescriptors = 2;
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	//ディスクリプタヒープの生成
	hr = m_device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&m_mcbvHeap));
	if (FAILED(hr)) {
		std::cout << "Failed to Create descHeapDesc\n";
		return hr;
	}
	//ディスクリプタヒープのスタート位置(CPU側)の取得
	D3D12_CPU_DESCRIPTOR_HANDLE handleOffset = m_mcbvHeap->GetCPUDescriptorHandleForHeapStart();
	//定数バッファビューの設定
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = m_mconstBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = sizeof(MatrixData);
	//定数バッファビューの生成
	m_device->CreateConstantBufferView(&cbvDesc, handleOffset);
	handleOffset.ptr += m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	cbvDesc.BufferLocation = m_sconstBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = sizeof(ShadowMatrixData);
	m_device->CreateConstantBufferView(&cbvDesc, handleOffset);

	return S_OK;
}

HRESULT DirectXController::CreateShaders() {
	
	if (FAILED(m_mvertexShader.CreateShader(L"VertexShader.hlsl", "main", "vs_5_0", m_errorBlob))) {
		return ErrorMessage("Failed to CreateShader");
	}
	if (FAILED(m_mpixelShader.CreateShader(L"PixelShader.hlsl", "main", "ps_5_0", m_errorBlob))) {
		return ErrorMessage("Failed to CreateShader");
	}
	if (FAILED(m_fvertexShader.CreateShader(L"finalRenderVertexShader.hlsl", "main", "vs_5_0", m_errorBlob))) {
		return ErrorMessage("Failed to CreateShader");
	}
	if (FAILED(m_fpixelShader.CreateShader(L"finalRenderPixelShader.hlsl", "main", "ps_5_0", m_errorBlob))) {
		return ErrorMessage("Failed to CreateShader");
	}
	if (FAILED(m_svertexShader.CreateShader(L"shadowVertexShader.hlsl", "main", "vs_5_0", m_errorBlob))) {
		return ErrorMessage("Failed to CreateShader");
	}
	return S_OK;
}

HRESULT DirectXController::CreateShadowMapGraphicsPipeLine() {
	HRESULT hr;
	//ディスクリプタテーブルの設定
	D3D12_DESCRIPTOR_RANGE desTbRange[2] = {};
	//定数バッファのディスクリプタレンジ
	desTbRange[0].NumDescriptors = 1;
	desTbRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	desTbRange[0].BaseShaderRegister = 0;
	desTbRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	//テクスチャバッファのディスクリプタレンジ
	desTbRange[1].NumDescriptors = 1;
	desTbRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	desTbRange[1].BaseShaderRegister = 0;
	desTbRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	//ルートパラメータの設定(レンジ1)
	D3D12_ROOT_PARAMETER rootParam[3] = {};
	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParam[0].DescriptorTable.pDescriptorRanges = &desTbRange[0];
	rootParam[0].DescriptorTable.NumDescriptorRanges = 1;
	//ルートパラメータの設定(レンジ2)
	rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParam[1].DescriptorTable.pDescriptorRanges = &desTbRange[1];
	rootParam[1].DescriptorTable.NumDescriptorRanges = 1;
	//ルートシグネチャの設定
	D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
	rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSigDesc.pParameters = rootParam;
	rootSigDesc.NumParameters = 2;
	rootSigDesc.pStaticSamplers = nullptr;// &samplerDesc;
	rootSigDesc.NumStaticSamplers = 0;
	hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &m_srootSig, &m_errorBlob);
	if (FAILED(hr)) {
		std::cout << "Failed to SealizeRootSignature\n";
		return hr;
	}
	//Create RootSignature
	hr = m_device->CreateRootSignature(0, m_srootSig->GetBufferPointer(), m_srootSig->GetBufferSize(), IID_PPV_ARGS(&m_srootSignature));
	if (FAILED(hr)) {
		std::cout << "Failed to CreateRootSignature\n";
		return hr;
	}

	/*-----グラフィックパイプラインステートの生成-----*/
	//Describe InputLayout
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{
			"POSITION", 0,DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};
	/*-----GraphicsPipeLineStateの定義-----*/
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gPipeLine = {};
	//RootSignatureの設定
	gPipeLine.pRootSignature = m_srootSignature;
	//Shaderの設定
	gPipeLine.VS.pShaderBytecode = m_svertexShader.GetBufferPointer();
	gPipeLine.VS.BytecodeLength = m_svertexShader.GetBufferSize();
	gPipeLine.PS.pShaderBytecode = nullptr;// m_mpixelShader.GetBufferPointer();
	gPipeLine.PS.BytecodeLength = 0;// m_mpixelShader.GetBufferSize();
	//SampleStateとRasterizerStateの設定
	gPipeLine.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	gPipeLine.RasterizerState.MultisampleEnable = false;
	gPipeLine.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	gPipeLine.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	gPipeLine.RasterizerState.DepthClipEnable = true;
	//BlendStateの設定
	gPipeLine.BlendState.AlphaToCoverageEnable = false;
	gPipeLine.BlendState.IndependentBlendEnable = false;
	D3D12_RENDER_TARGET_BLEND_DESC rtBlendDesc = {};
	rtBlendDesc.BlendEnable = false;
	rtBlendDesc.LogicOpEnable = false;
	rtBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	gPipeLine.BlendState.RenderTarget[0] = rtBlendDesc;
	//InputLayoutの設定
	gPipeLine.InputLayout.pInputElementDescs = inputLayout;
	gPipeLine.InputLayout.NumElements = _countof(inputLayout);
	//PrimitiveTopologyの設定
	gPipeLine.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	gPipeLine.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//RenderTargetの設定
	gPipeLine.NumRenderTargets = 0;
	//gPipeLine.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	//Samplerの設定
	gPipeLine.SampleDesc.Count = 1;
	//DepthStencilの設定
	gPipeLine.DepthStencilState.DepthEnable = true;
	gPipeLine.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	gPipeLine.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	gPipeLine.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	//GraphicPipeLineStateの生成
	hr = m_device->CreateGraphicsPipelineState(&gPipeLine, IID_PPV_ARGS(&m_spipeLineState));
	if (FAILED(hr)) {
		std::cout << "Failed to Create GraphicsPipelineState\n";
		return hr;
	}



	return S_OK;
}

HRESULT DirectXController::CreateFinalGraphicsPipeLine() {
	HRESULT hr;
	/*
	最終出力結果を張り付けるポリゴンを表示するためのグラフィクスパイプライン
	*/
	/*-----インプットレイアウトの定義-----*/
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ 
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0	
		},
		{
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};

	/*-----GraphicsPipeLineStateの定義-----*/
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gPipeLine = {};
	//インプットレイアウトの設定
	gPipeLine.InputLayout.NumElements = _countof(inputLayout);
	gPipeLine.InputLayout.pInputElementDescs = inputLayout;
	//シェーダの設定
	gPipeLine.VS.pShaderBytecode = m_fvertexShader.GetBufferPointer();
	gPipeLine.VS.BytecodeLength = m_fvertexShader.GetBufferSize();
	gPipeLine.PS.pShaderBytecode = m_fpixelShader.GetBufferPointer();
	gPipeLine.PS.BytecodeLength = m_fpixelShader.GetBufferSize();
	//SampleStateとRasterizerStateの設定
	gPipeLine.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	gPipeLine.RasterizerState.MultisampleEnable = false;
	gPipeLine.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	gPipeLine.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	gPipeLine.RasterizerState.DepthClipEnable = true;
	//BlendStateの設定	
	D3D12_RENDER_TARGET_BLEND_DESC rtBlendDesc = {};
	rtBlendDesc.BlendEnable = false;
	rtBlendDesc.LogicOpEnable = false;
	rtBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	gPipeLine.BlendState.RenderTarget[0] = rtBlendDesc;
	gPipeLine.BlendState.AlphaToCoverageEnable = false;
	gPipeLine.BlendState.IndependentBlendEnable = false;

	//PrimitiveTopologyの設定
	gPipeLine.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	gPipeLine.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//RenderTargetの設定
	gPipeLine.NumRenderTargets = 1;
	gPipeLine.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	//Samplerの設定
	gPipeLine.SampleDesc.Count = 1;
	gPipeLine.SampleDesc.Quality = 0;
	gPipeLine.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	/*-----ルートシグネチャの設定-----*/
	D3D12_DESCRIPTOR_RANGE range = {};
	range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range.BaseShaderRegister = 0;
	range.NumDescriptors = 1;
	D3D12_ROOT_PARAMETER rootParam = {};
	rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParam.DescriptorTable.pDescriptorRanges = &range;
	rootParam.DescriptorTable.NumDescriptorRanges = 1;
	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
	rootSigDesc.NumParameters = 1;
	rootSigDesc.pParameters = &rootParam;
	rootSigDesc.NumStaticSamplers = 1;
	rootSigDesc.pStaticSamplers = &samplerDesc;
	rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &m_frootSig, &m_errorBlob);
	if (FAILED(hr)) {
		std::cout << "Failed to Serialize finalRenderRootSignature" << std::endl;
		return hr;
	}
	hr = m_device->CreateRootSignature(0, m_frootSig->GetBufferPointer(), m_frootSig->GetBufferSize(), IID_PPV_ARGS(&m_frootsignature));
	if (FAILED(hr)) {
		std::cout << "Failed to Create finalRenderRootSignature" << std::endl;
		return hr;
	}
	gPipeLine.pRootSignature = m_frootsignature;
	//GraphicPipeLineStateの生成
	hr = m_device->CreateGraphicsPipelineState(&gPipeLine, IID_PPV_ARGS(&m_fpipeLineState));
	if (FAILED(hr)) {
		std::cout << "Failed to Create finalGraphicsPipelineState\n";
		return hr;
	}
	return S_OK;
}

HRESULT DirectXController::SetGraphicsPipeLine() {
	HRESULT hr;

	//サンプラーの設定
	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	//ディスクリプタテーブルの設定
	D3D12_DESCRIPTOR_RANGE desTbRange[3] = {};
	//定数バッファのディスクリプタレンジ
	desTbRange[0].NumDescriptors = 1;
	desTbRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	desTbRange[0].BaseShaderRegister = 0;
	desTbRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	//マテリアルバッファのディスクリプタレンジ
	desTbRange[1].NumDescriptors = 1;
	desTbRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	desTbRange[1].BaseShaderRegister = 1;
	desTbRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	//テクスチャバッファのディスクリプタレンジ
	desTbRange[2].NumDescriptors = 3;
	//desTbRange[2].NumDescriptors = 1;
	desTbRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	desTbRange[2].BaseShaderRegister = 0;
	desTbRange[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	//ルートパラメータの設定(レンジ1)
	D3D12_ROOT_PARAMETER rootParam[3] = {};
	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParam[0].DescriptorTable.pDescriptorRanges = &desTbRange[0];
	rootParam[0].DescriptorTable.NumDescriptorRanges = 1;
	//ルートパラメータの設定(レンジ2)
	rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParam[1].DescriptorTable.pDescriptorRanges = &desTbRange[1];
	rootParam[1].DescriptorTable.NumDescriptorRanges = 1;
	//ルートパラメータの設定(レンジ3)
	rootParam[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParam[2].DescriptorTable.pDescriptorRanges = &desTbRange[2];
	rootParam[2].DescriptorTable.NumDescriptorRanges = 1;
	//ルートシグネチャの設定
	D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
	rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSigDesc.pParameters = rootParam;
	rootSigDesc.NumParameters = 3;
	rootSigDesc.pStaticSamplers = &samplerDesc;
	rootSigDesc.NumStaticSamplers = 1;
	hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &m_mrootSig, &m_errorBlob);
	if (FAILED(hr)) {
		std::cout << "Failed to SealizeRootSignature\n";
		return hr;
	}
	//Create RootSignature
	hr = m_device->CreateRootSignature(0, m_mrootSig->GetBufferPointer(), m_mrootSig->GetBufferSize(), IID_PPV_ARGS(&m_mrootsignature));
	if (FAILED(hr)) {
		std::cout << "Failed to CreateRootSignature\n";
		return hr;
	}

	/*-----グラフィックパイプラインステートの生成-----*/
	//Describe InputLayout
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{
			"POSITION", 0,DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};
	/*-----GraphicsPipeLineStateの定義-----*/
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gPipeLine = {};
	//RootSignatureの設定
	gPipeLine.pRootSignature = m_mrootsignature;
	//Shaderの設定
	gPipeLine.VS.pShaderBytecode = m_mvertexShader.GetBufferPointer();
	gPipeLine.VS.BytecodeLength = m_mvertexShader.GetBufferSize();
	gPipeLine.PS.pShaderBytecode = m_mpixelShader.GetBufferPointer();
	gPipeLine.PS.BytecodeLength = m_mpixelShader.GetBufferSize();
	//SampleStateとRasterizerStateの設定
	gPipeLine.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	gPipeLine.RasterizerState.MultisampleEnable = false;
	gPipeLine.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	gPipeLine.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	gPipeLine.RasterizerState.DepthClipEnable = true;
	//BlendStateの設定
	gPipeLine.BlendState.AlphaToCoverageEnable = false;
	gPipeLine.BlendState.IndependentBlendEnable = false;
	D3D12_RENDER_TARGET_BLEND_DESC rtBlendDesc = {};
	rtBlendDesc.BlendEnable = false;
	rtBlendDesc.LogicOpEnable = false;
	rtBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	gPipeLine.BlendState.RenderTarget[0] = rtBlendDesc;
	//InputLayoutの設定
	gPipeLine.InputLayout.pInputElementDescs = inputLayout;
	gPipeLine.InputLayout.NumElements = _countof(inputLayout);
	//PrimitiveTopologyの設定
	gPipeLine.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	gPipeLine.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//RenderTargetの設定
	gPipeLine.NumRenderTargets = 1;
	gPipeLine.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	//Samplerの設定
	gPipeLine.SampleDesc.Count = 1;
	//DepthStencilの設定
	gPipeLine.DepthStencilState.DepthEnable = true;
	gPipeLine.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	gPipeLine.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	gPipeLine.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	//GraphicPipeLineStateの生成
	hr = m_device->CreateGraphicsPipelineState(&gPipeLine, IID_PPV_ARGS(&m_mpipeLineState));
	if (FAILED(hr)) {
		std::cout << "Failed to Create GraphicsPipelineState\n";
		return hr;
	}



	return S_OK;
}

HRESULT DirectXController::DrawFromLight(Light& light) {
	D3D12_CPU_DESCRIPTOR_HANDLE dsvH = m_sdsvHeap->GetCPUDescriptorHandleForHeapStart();
	std::cout << dsvH.ptr << std::endl;
	
	//リソースバリアの更新(シェーダリソースからレンダーターゲット)
	m_cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_shadowBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE));

	//深度/ステンシルバッファのクリア
	m_cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	//ここから、オブジェクトの数だけループ
	for (int i = 0; i < m_objsOBJ.size(); i++) {
		//パイプラインとルートシグネチャをセット
		m_cmdList->SetPipelineState(m_spipeLineState);
		m_cmdList->SetGraphicsRootSignature(m_srootSignature);

		//レンダーターゲットと深度/ステンシルバッファをセット
		m_cmdList->OMSetRenderTargets(0, nullptr, false, &dsvH);

		//ビューポートとシザー矩形をセット
		m_cmdList->RSSetViewports(1, &m_vp);
		m_cmdList->RSSetScissorRects(1, &m_sr);

		//プリミティブトポロジをセット
		m_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//オブジェクトを描画
		(m_smapMatrix)->w = m_objsOBJ[i]->m_wMatrix;
		for (int j = 0; j < m_objsOBJ[i]->m_obj.size(); j++) {
			m_objsOBJ[i]->m_obj[j]->Draw(m_cmdList, m_mcbvHeap, m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
		}


		//コマンドリストを閉じる
		m_cmdList->Close();

		//コマンドを実行
		ID3D12CommandList* cmdlists[] = { m_cmdList };
		m_cmdQueue->ExecuteCommandLists(1, cmdlists);

		//コマンドの実行終了を待つ
		m_cmdQueue->Signal(m_fence, ++m_fenceVal);
		if (m_fence->GetCompletedValue() != m_fenceVal) {
			auto event = CreateEvent(nullptr, false, false, nullptr);
			m_fence->SetEventOnCompletion(m_fenceVal, event);
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}

		//コマンドのリセット
		m_cmdAllocator->Reset();
		m_cmdList->Reset(m_cmdAllocator, nullptr);
		
	}
	//リソースバリアの更新
	m_cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_shadowBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	return S_OK;
}

HRESULT DirectXController::DrawFromCamera(Camera& camera) {
	//CPUディスクリプターヒープスタート位置の取得
	D3D12_CPU_DESCRIPTOR_HANDLE rtvH = m_mrtvHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_CPU_DESCRIPTOR_HANDLE dsvH = m_mdsvHeap->GetCPUDescriptorHandleForHeapStart();

	//リソースバリアの更新(シェーダリソースからレンダーターゲット)
	m_cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_mrenderBuffers[0], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));

	//レンダーターゲットのクリア
	float clearColor[] = { 0.5f,0.5f,0.7f,1.0f };
	m_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

	//深度/ステンシルバッファのクリア
	m_cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	UpdateViewMatrix(camera, 0);
	//ここから、オブジェクトの数だけループ
	for (int i = 0; i < m_objsOBJ.size(); i++) {
		//パイプラインとルートシグネチャをセット
		m_cmdList->SetPipelineState(m_mpipeLineState);
		m_cmdList->SetGraphicsRootSignature(m_mrootsignature);

		//レンダーターゲットと深度/ステンシルバッファをセット
		m_cmdList->OMSetRenderTargets(1, &rtvH, false, &dsvH);

		//ビューポートとシザー矩形をセット
		m_cmdList->RSSetViewports(1, &m_vp);
		m_cmdList->RSSetScissorRects(1, &m_sr);

		//シャドウテクスチャをセット
		D3D12_GPU_DESCRIPTOR_HANDLE handle = m_ssrvHeap->GetGPUDescriptorHandleForHeapStart();
		m_cmdList->SetDescriptorHeaps(1, &m_ssrvHeap);
		m_cmdList->SetGraphicsRootDescriptorTable(2, handle);

		//プリミティブトポロジをセット
		m_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//オブジェクトを描画
		m_mmapMatrix->w = m_objsOBJ[i]->m_wMatrix;
		for (int j = 0; j < m_objsOBJ[i]->m_obj.size(); j++) {
			m_objsOBJ[i]->m_obj[j]->Draw(m_cmdList, m_mcbvHeap, 0);
		}
		//コマンドリストを閉じる
		m_cmdList->Close();

		//コマンドを実行
		ID3D12CommandList* cmdlists[] = { m_cmdList };
		m_cmdQueue->ExecuteCommandLists(1, cmdlists);

		//コマンドの実行終了を待つ
		m_cmdQueue->Signal(m_fence, ++m_fenceVal);
		if (m_fence->GetCompletedValue() != m_fenceVal) {
			auto event = CreateEvent(nullptr, false, false, nullptr);
			m_fence->SetEventOnCompletion(m_fenceVal, event);
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}

		//コマンドのリセット
		m_cmdAllocator->Reset();
		m_cmdList->Reset(m_cmdAllocator, nullptr);
		//}
	}
	//リソースバリアの更新
	m_cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_mrenderBuffers[0], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	return S_OK;
}

HRESULT DirectXController::finalDraw() {
	//バックバッファのインデックス取得
	UINT bbID = m_swapchain->GetCurrentBackBufferIndex();

	//CPUディスクリプタヒープスタート位置の取得
	D3D12_CPU_DESCRIPTOR_HANDLE rtvH = m_frtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += bbID * m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE dsvH = m_mdsvHeap->GetCPUDescriptorHandleForHeapStart();
	
	//リソースバリアの更新
	m_cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_backBuffers[bbID], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	//レンダーターゲットのクリア
	float clearColor[] = { 0.5f,0.5f,0.5f,1.0f };
	m_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

	//深度/ステンシルバッファのクリア
	m_cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	
	//パイプラインとルートシグネチャを設定
	m_cmdList->SetPipelineState(m_fpipeLineState);
	m_cmdList->SetGraphicsRootSignature(m_frootsignature);

	//レンダーターゲットを設定
	m_cmdList->OMSetRenderTargets(1, &rtvH, false, &dsvH);

	//ビューポートとシザー矩形の設定
	m_cmdList->RSSetViewports(1, &m_vp);
	m_cmdList->RSSetScissorRects(1, &m_sr);

	//ディスクリプタヒープとディスクリプタを設定
	D3D12_GPU_DESCRIPTOR_HANDLE handle = m_msrvHeap->GetGPUDescriptorHandleForHeapStart();
	m_cmdList->SetDescriptorHeaps(1, &m_msrvHeap);
	m_cmdList->SetGraphicsRootDescriptorTable(0, handle);

	//プリミティブトポロジの設定
	m_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//頂点のセット
	m_cmdList->IASetVertexBuffers(0, 1, &m_frenderVbv);
	
	//描画
	m_cmdList->DrawInstanced(4, 1, 0, 0);

	//リソースバリアの更新
	m_cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_backBuffers[bbID], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	//コマンドリストを閉じる
	m_cmdList->Close();

	//コマンドの実行
	ID3D12CommandList* cmdlists[] = { m_cmdList };
	m_cmdQueue->ExecuteCommandLists(1, cmdlists);

	//コマンドの実行終了を待つ
	m_cmdQueue->Signal(m_fence, ++m_fenceVal);
	if (m_fence->GetCompletedValue() != m_fenceVal) {
		auto event = CreateEvent(nullptr, false, false, nullptr);
		m_fence->SetEventOnCompletion(m_fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}

	//コマンドのリセット
	m_cmdAllocator->Reset();
	m_cmdList->Reset(m_cmdAllocator, nullptr);

	//バックバッファの内容を表示
	m_swapchain->Present(1, 0);

	return S_OK;
}

HRESULT DirectXController::LoadObject(const char* objName) {
	HRESULT hr;
	PathController pc;
	if (m_LoadedObjCount < MAX_OBJECT_COUNT) {
		m_objsOBJ.push_back(new DX12ObjectFormatOBJ()); //一つ要素を確保する
		if (FAILED(m_objsOBJ[m_objsOBJ.size() - 1]->LoadVertexFromOBJFile(objName, m_device))) {
			delete(m_objsOBJ[m_objsOBJ.size() - 1]);
			m_objsOBJ.erase(m_objsOBJ.end());

			return E_FAIL;
		}

		//成功したらロード済みオブジェクト数を増やす
		m_LoadedObjCount++;
	}

	return S_OK;
}

HRESULT DirectXController::DeleteObject(int objIdx) {
	delete(m_objsOBJ[objIdx]);
	m_objsOBJ.erase(m_objsOBJ.begin() + objIdx);

	return S_OK;
}

HRESULT DirectXController::UpdateObjTransform(HWND hwnd[9], int offset, XMFLOAT3& objData) {
	float data[3];

	for (int i = 0; i < 3; i++) {
		LPTSTR dataTxt = (LPTSTR)calloc((GetWindowTextLength(hwnd[i + offset]) + 1), sizeof(TCHAR)); //get EditBox's Text
		GetWindowText(hwnd[i + offset], dataTxt, GetWindowTextLength(hwnd[i + offset]) + 1);
		try {
			data[i] = std::stof(dataTxt); //if dataTxt is not value, data[i] is setted 0
		}
		catch (std::exception& e) {
			data[i] = 0.0f;
		}
	}
	objData.x = data[0];
	objData.y = data[1];
	objData.z = data[2];

	return S_OK;
}

HRESULT DirectXController::UpdateObjTransform(HWND hwnd[9], float value[3], int offset, XMFLOAT3& objData) {
	objData.x += value[0];
	objData.y += value[1];
	objData.z += value[2];

	return S_OK;
}

HRESULT DirectXController::UpdateWorldMatrix(Object& obj, int objIndex) {
	XMMATRIX posMat = XMMatrixTranslation(obj.transform.position.x, obj.transform.position.y, obj.transform.position.z);
	XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(obj.transform.rotation.x * XM_PI / 180, obj.transform.rotation.y * XM_PI / 180, obj.transform.rotation.z * XM_PI / 180);
	XMMATRIX sizMat = XMMatrixScaling(obj.transform.size.x, obj.transform.size.y, obj.transform.size.z);

	XMMATRIX worldMat = sizMat * rotMat * posMat; //Create New Matrix to hand mapMatrix

	(m_mmapMatrix + objIndex)->w = worldMat;

	return S_OK;
}

HRESULT DirectXController::UpdateViewMatrix(Camera& camera, int objIndex) {
	m_mmapMatrix->v_camera = camera.viewMatrix; //set Camera's viewMatrix to mapMatrix
	m_mmapMatrix->eye = camera.pos;
	return S_OK;
}

void DirectXController::EnableDebugLayer() {
	ID3D12Debug* debugLayer = nullptr;
	auto result = D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));
	debugLayer->EnableDebugLayer();
	debugLayer->Release();
}

void DirectXController::Release() {
	if (m_device)m_device->Release();
	if (m_factory)m_factory->Release();
	if (m_swapchain)m_swapchain->Release();

	if (m_cmdAllocator)m_cmdAllocator->Release();
	if (m_cmdList)m_cmdList->Release();
	if (m_cmdQueue)m_cmdQueue->Release();

	if (m_frtvHeap)m_frtvHeap->Release();
	if (m_mrtvHeap)m_mrtvHeap->Release();
	if (m_mdsvHeap)m_mdsvHeap->Release();
	if (m_sdsvHeap)m_sdsvHeap->Release();
	if (m_ssrvHeap)m_ssrvHeap->Release();
	if (m_mcbvHeap)m_mcbvHeap->Release();

	/*if (m_mvertexShader)m_mvertexShader->Release();
	if (m_mpixelShader)m_mpixelShader->Release();*/
	if (m_errorBlob)m_errorBlob->Release();
	if (m_mrootSig)m_mrootSig->Release();  

	if (m_mpipeLineState)m_mpipeLineState->Release();
	if (m_spipeLineState)m_spipeLineState->Release();
	
	if (m_mrootsignature)m_mrootsignature->Release();   

	for (int i = 0; i < RT_BUFFER_COUNT; i++) {
		if (m_backBuffers[i])m_backBuffers[i]->Release();
	}
	if (m_mdepthBuffer)m_mdepthBuffer->Release();
	if (m_mconstBuffer)m_mconstBuffer->Release();
	if (m_shadowBuffer)m_shadowBuffer->Release();
	if (m_sconstBuffer)m_sconstBuffer->Release();
	for (int i = 0; i < RT_MULTIPASS_COUNT; i++) {
		if (m_mrenderBuffers[i])m_mrenderBuffers[i]->Release();
	}
	if (m_frenderPolygon)m_frenderPolygon->Release();

	if (m_fence)m_fence->Release();

}
