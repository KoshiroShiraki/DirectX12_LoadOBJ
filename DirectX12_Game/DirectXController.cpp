/*
☆DirectXController.h
DirectX制御クラス
*/
#pragma once
#include"DirectXController.h"

DirectXController::DirectXController() {
}

DirectXController::~DirectXController() {
}

HRESULT DirectXController::InitD3D(HWND hwnd) {
	HRESULT hr;

#ifdef _DEBUG
	EnableDebugLayer();
#endif

	/*-----Factoryの生成-----*/
	hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&factory));
	if (FAILED(hr)) {
		std::cout << "Failed to Create Factory\n";
		return hr;
	}

	/*-----Deviceの生成-----*/
	//アダプタの列挙
	std::vector <IDXGIAdapter*> adapters;
	IDXGIAdapter* tmpAdapter = nullptr;
	for (int i = 0; factory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; i++) {
		adapters.push_back(tmpAdapter);
	}
	//アダプタの決定
	for (auto adpt : adapters) {
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);
		std::wstring strDesc = adesc.Description;
		if (strDesc.find(L"NVIDIA") != std::string::npos) {
			tmpAdapter = adpt;
			std::wcout << "アダプタ : " << strDesc << "\n";
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
	//Deviceの生成
	D3D_FEATURE_LEVEL featureLevel;
	for (auto lv : levels) {
		hr = D3D12CreateDevice(tmpAdapter, lv, IID_PPV_ARGS(&device));
		if (SUCCEEDED(hr)) {
			featureLevel = lv;
			std::cout << "機能レベル : " << lv << "\n";
			break;
		}
	}
	if (FAILED(hr)) {
		std::cout << "Failed to Create Device\n";
		return hr;
	}

	/*-----CommandAllocatorの生成-----*/
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAllocator));
	if (FAILED(hr)) {
		std::cout << "Falied to Create CommandAllocator\n";
		return hr;
	}

	/*-----CommandListの生成-----*/
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAllocator, nullptr, IID_PPV_ARGS(&cmdList));
	if (FAILED(hr)) {
		std::cout << "Failed to Create CommandList\n";
		return hr;
	}

	/*-----CommandQueueの生成-----*/
	//CommandQueueの定義
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	//CommandQueueの生成
	hr = device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&cmdQueue));
	if (FAILED(hr)) {
		std::cout << "Failed to Create CommandQueue\n";
		return hr;
	}

	/*-----SwapChainの生成-----*/
	//SwapChainの定義
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.Width = window_Width;
	swapchainDesc.Height = window_Height;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	//SwapChainの生成
	backBuffers.resize(swapchainDesc.BufferCount);
	hr = factory->CreateSwapChainForHwnd(cmdQueue, hwnd, &swapchainDesc, nullptr, nullptr, (IDXGISwapChain1**)&swapchain);
	if (FAILED(hr)) {
		std::cout << "Failed to Create SwapChain";
		return hr;
	}

	/*-----RenderTargetViewの生成-----*/
	//DescriptorHeapの定義
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	//DescriptorHeapの生成
	hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeap));
	if (FAILED(hr)) {
		return 0;
	}
	//RenderTargetViewの生成
	D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (int i = 0; i < swapchainDesc.BufferCount; i++) {
		hr = swapchain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i]));
		device->CreateRenderTargetView(backBuffers[i], nullptr, handle);
		handle.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	/*-----fenceの生成-----*/
	hr = device->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	if (FAILED(hr)) {
		std::cout << "Failed to Create Fence\n";
		return hr;
	}

	/*-----DepthStencilBufferの生成-----*/
	//Depth/StencilBufferの定義
	D3D12_RESOURCE_DESC depthResDesc = {};
	depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResDesc.Width = window_Width;
	depthResDesc.Height = window_Height;
	depthResDesc.DepthOrArraySize = 1;
	depthResDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthResDesc.SampleDesc.Count = 1;
	depthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	//Depth/StencilBufferのHeapProperty
	D3D12_HEAP_PROPERTIES depthHeapProp = {};
	depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	depthHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	depthHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	//ClearValue
	D3D12_CLEAR_VALUE depthClearValue = {};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	//Depth/StencilBufferの生成
	hr = device->CreateCommittedResource(&depthHeapProp, D3D12_HEAP_FLAG_NONE, &depthResDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue, IID_PPV_ARGS(&depthBuffer));
	if (FAILED(hr)) {
		std::cout << "Failed to Create DepthStencilBuffer\n";
		return hr;
	}

	/*-----DepthStencilViewの生成-----*/
	//DescriptorHeapの定義
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hr = device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap));
	if (FAILED(hr)) {
		std::cout << "Failed to Create dsvHeap\n";
	}
	//Depth/StencilViewの生成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	device->CreateDepthStencilView(depthBuffer, &dsvDesc, dsvHeap->GetCPUDescriptorHandleForHeapStart());

	return S_OK;
}

HRESULT DirectXController::CreateResources() {
	HRESULT hr;

#ifdef _DEBUG
	EnableDebugLayer();
#endif
	/*-----OBJデータの読み込み-----*/
	car.LoadOBJData("OBJ/99-intergalactic_spaceship-obj/Intergalactic_Spaceship-(Wavefront)", device);

	/*-----PMDデータの読み込み-----*/
	//miku.LoadPMDData("初音ミク", device);

	/*-----ConstantBufferの生成-----*/
	//ワールド行列の生成
	worldMatrix = DirectX::XMMatrixIdentity();
	//ビュー行列の生成
	DirectX::XMFLOAT3 eye(0, 5, -10);
	DirectX::XMFLOAT3 target(0, 0, 0);
	DirectX::XMFLOAT3 up(0, 1, 0);
	camera.InitCamera(eye, target, up);
	//プロジェクション行列の生成
	projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, static_cast<float>(window_Width) / static_cast<float>(window_Height), 1.0f, 100.0f);
	//ResourcDescの定義
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = (sizeof(MatrixData) + 0xff) & ~0xff;
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count = 1;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//HeapPropertyの設定
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,   
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuffer)
	);
	//ConstantBufferの生成
	hr = constBuffer->Map(0, nullptr, (void**)&mapMatrix);
	if (FAILED(hr)) {
		std::cout << "Failed to Map constBuffer\n";
		return hr;
	}
	mapMatrix->w = worldMatrix;
	mapMatrix->v = camera.viewMatrix;
	mapMatrix->p = projectionMatrix;
	mapMatrix->eye = eye;

	/*-----ConstantBufferViewの生成-----*/
	//DescriptorHeapの定義
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NodeMask = 0;
	descHeapDesc.NumDescriptors = 1;
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	//DescriptorHeapの生成
	hr = device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&basicDescHeap));
	if (FAILED(hr)) {
		std::cout << "Failed to Create descHeapDesc\n";
		return hr;
	}
	//DescriptorHeapのハンドル位置取得
	auto basicHeapHandle = basicDescHeap->GetCPUDescriptorHandleForHeapStart();
	//ConstantBufferViewの定義
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = constBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = constBuffer->GetDesc().Width;
	//ConstantBufferViewの生成
	device->CreateConstantBufferView(&cbvDesc, basicHeapHandle);

	return S_OK;
}

HRESULT DirectXController::CreateShaders() {
	//VertexShaderのコンパイル
	HRESULT hr;
	hr = D3DCompileFromFile(L"VertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vertexShader, &errorBlob);
	if (FAILED(hr)) {
		if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			std::cout << "Not Found File\n";
		}
		else {
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";

			std::cout << "Failed to Compile VertexShader\n";
			return 0;
		}
	}

	//PixelShaderのコンパイル
	hr = D3DCompileFromFile(L"PixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pixelShader, &errorBlob);
	if (FAILED(hr)) {
		if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			std::cout << "Not Found File\n";
		}
		else {
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";

			std::cout << "Failed to Compile PixelShader\n";
			return hr;
		}
	}

	return S_OK;
}

HRESULT DirectXController::SetGraphicsPipeLine() {
	HRESULT hr;

	/*-----RootSignatureの生成-----*/
	//Samplerの定義
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
	//DescriptorTableの生成
	D3D12_DESCRIPTOR_RANGE desTbRange[1] = {};
	//定数バッファ(wvp行列)
	desTbRange[0].NumDescriptors = 1;
	desTbRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	desTbRange[0].BaseShaderRegister = 0;
	desTbRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	//RootPramaterの生成
	D3D12_ROOT_PARAMETER rootParam[1] = {};
	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParam[0].DescriptorTable.pDescriptorRanges = &desTbRange[0];
	rootParam[0].DescriptorTable.NumDescriptorRanges = 1;
	//RootSignatureの定義
	D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
	rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSigDesc.pParameters = rootParam;
	rootSigDesc.NumParameters = 1;
	rootSigDesc.pStaticSamplers = &samplerDesc;
	rootSigDesc.NumStaticSamplers = 1;
	hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSig, &errorBlob);
	if (FAILED(hr)) {
		std::cout << "Failed to SealizeRootSignature\n";
		return hr;
	}
	//RootSignatureの生成
	hr = device->CreateRootSignature(0, rootSig->GetBufferPointer(), rootSig->GetBufferSize(), IID_PPV_ARGS(&rootsignature));
	if (FAILED(hr)) {
		std::cout << "Failed to CreateRootSignature\n";
		return hr;
	}

	/*-----GraphicsPipeLineStateの生成-----*/
	//InputLayoutの定義
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
		{
			"BONE_NO", 0, DXGI_FORMAT_R16G16_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"WEIGHT", 0, DXGI_FORMAT_R8_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"EDGE_FLG", 0, DXGI_FORMAT_R8_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};
	/*-----GraphicsPipeLineStateの定義-----*/
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gPipeLine = {};
	//RootSignatureの設定
	gPipeLine.pRootSignature = rootsignature;
	//Shaderの設定
	gPipeLine.VS.pShaderBytecode = vertexShader->GetBufferPointer();
	gPipeLine.VS.BytecodeLength = vertexShader->GetBufferSize();
	gPipeLine.PS.pShaderBytecode = pixelShader->GetBufferPointer();
	gPipeLine.PS.BytecodeLength = pixelShader->GetBufferSize();
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
	gPipeLine.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
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
	hr = device->CreateGraphicsPipelineState(&gPipeLine, IID_PPV_ARGS(&PipeLineState));
	if (FAILED(hr)) {
		std::cout << "Failed to Create GraphicsPipelineState\n";
		return hr;
	}

	return S_OK;
}

HRESULT DirectXController::Draw() {
	//ViewPortの設定
	D3D12_VIEWPORT vp = {};
	vp.Width = window_Width;
	vp.Height = window_Height;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.MaxDepth = 1.0f;
	vp.MinDepth = 0.0f;

	//Scissor矩形の設定
	D3D12_RECT sr = {};
	sr.top = 0;
	sr.left = 0;
	sr.right = sr.left + window_Width;
	sr.bottom = sr.top + window_Height;

	/*-----コマンド命令-----*/
	//RenderTargetViewの取得
	angle += 0.001f;
	worldMatrix = DirectX::XMMatrixRotationY(angle);
	mapMatrix->w = worldMatrix;

	auto bbID = swapchain->GetCurrentBackBufferIndex();

	D3D12_RESOURCE_BARRIER BarrierDesc = {};
	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	BarrierDesc.Transition.pResource = backBuffers[bbID];
	BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	cmdList->ResourceBarrier(1, &BarrierDesc);

	cmdList->SetPipelineState(PipeLineState);


	//RenderTarget�̎w��
	auto rtvH = rtvHeap->GetCPUDescriptorHandleForHeapStart();
	auto dsvH = dsvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += bbID * device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	cmdList->OMSetRenderTargets(1, &rtvH, false, &dsvH);

	//Depth/StencilBuffer�̃N���A
	cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	//RenderTarget�̃N���A
	float clearColor[] = { 0.5f,0.0f,0.0f,0.0f };
	cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

	cmdList->RSSetViewports(1, &vp);
	cmdList->RSSetScissorRects(1, &sr);

	cmdList->SetGraphicsRootSignature(rootsignature);

	cmdList->SetDescriptorHeaps(1, &basicDescHeap);

	auto heapHandle = basicDescHeap->GetGPUDescriptorHandleForHeapStart();
	cmdList->SetGraphicsRootDescriptorTable(0, heapHandle);

	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->IASetVertexBuffers(0, 1, &car.vbView);
	cmdList->IASetIndexBuffer(&car.ibView);

	cmdList->DrawIndexedInstanced(car.indices.size(), 1, 0, 0, 0);

	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	cmdList->ResourceBarrier(1, &BarrierDesc);

	//���߂̃N���[�Y
	cmdList->Close();
	//���߂̎��s
	ID3D12CommandList* cmdlists[] = { cmdList };
	cmdQueue->ExecuteCommandLists(1, cmdlists);

	cmdQueue->Signal(fence, ++fenceVal);
	if (fence->GetCompletedValue() != fenceVal) {
		auto event = CreateEvent(nullptr, false, false, nullptr);
		fence->SetEventOnCompletion(fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}

	//�L���[�̃N���A
	cmdAllocator->Reset();
	cmdList->Reset(cmdAllocator, nullptr);

	//�t���b�v
	swapchain->Present(1, 0);
	
	return S_OK;
}

void DirectXController::EnableDebugLayer() {
	ID3D12Debug* debugLayer = nullptr;
	auto result = D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));
	debugLayer->EnableDebugLayer();
	debugLayer->Release();
}

void DirectXController::Release() {
	if (device)device->Release();
	if (factory)factory->Release();
	if (swapchain)swapchain->Release();

	if (cmdAllocator)cmdAllocator->Release();
	if (cmdList)cmdList->Release();
	if (cmdQueue)cmdQueue->Release();

	if (rtvHeap)rtvHeap->Release();
	if (dsvHeap)dsvHeap->Release();
	if (basicDescHeap)basicDescHeap->Release();

	if (vertexShader)vertexShader->Release();
	if (pixelShader)pixelShader->Release();
	if (errorBlob)errorBlob->Release();
	if (rootSig)rootSig->Release();  

	if (fence)fence->Release();

	if (PipeLineState)PipeLineState->Release();

	if (rootsignature)rootsignature->Release();   

	for (int i = 0; i < backBuffers.size(); i++) {
		if (backBuffers[i]) backBuffers[i]->Release();
	}
	if (depthBuffer)depthBuffer->Release();
	//if (vertexBuffer)vertexBuffer->Release();
	//if (indexBuffer)indexBuffer->Release();
	if (constBuffer)constBuffer->Release();
}
