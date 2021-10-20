#pragma once
#include"DirectXController.h"

DirectXController::DirectXController() {
	objs.resize(MAX_OBJECT_COUNT);
}

DirectXController::~DirectXController() {
}

HRESULT DirectXController::InitD3D(HWND hwnd) {
	HRESULT hr;

#ifdef _DEBUG
	EnableDebugLayer();
#endif

	/*-----Create Factory ... use for Create Swapchain, CommandList, ...-----*/
	hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&factory));
	if (FAILED(hr)) {
		std::cout << "Failed to Create Factory\n";
		return hr;
	}

	/*-----Create Device ... use for Crate Resource, DescriptorHeap, ...-----*/
	//enumerate Video Adapter
	std::vector <IDXGIAdapter*> adapters;
	IDXGIAdapter* tmpAdapter = nullptr;
	for (int i = 0; factory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; i++) {
		adapters.push_back(tmpAdapter);
	}
	//Set Adaptor
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
	//enumerate Feature Levels
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};
	//CreateDevice
	D3D_FEATURE_LEVEL featureLevel;
	for (auto lv : levels) {
		hr = D3D12CreateDevice(tmpAdapter, lv, IID_PPV_ARGS(&device));
		if (SUCCEEDED(hr)) {
			featureLevel = lv;
			std::cout << lv << std::endl;
			break;
		}
	}
	if (FAILED(hr)) {
		std::cout << "Failed to Create Device\n";
		return hr;
	}

	/*-----Create CommandAllocator-----*/
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAllocator));
	if (FAILED(hr)) {
		std::cout << "Falied to Create CommandAllocator\n";
		return hr;
	}

	/*-----Create CommandList-----*/
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAllocator, nullptr, IID_PPV_ARGS(&cmdList));
	if (FAILED(hr)) {
		std::cout << "Failed to Create CommandList\n";
		return hr;
	}

	/*-----CreateCommandQueue-----*/
	//Describe CommandQueue
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	//Create CommandQueue
	hr = device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&cmdQueue));
	if (FAILED(hr)) {
		std::cout << "Failed to Create CommandQueue\n";
		return hr;
	}

	/*-----Create SwapChain ... swap BackBuffer to FrontBuffer(this Application uses DoubleBuffering)-----*/
	//Describe SwapChain
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
	//Create SwapChain
	backBuffers.resize(swapchainDesc.BufferCount);
	hr = factory->CreateSwapChainForHwnd(cmdQueue, hwnd, &swapchainDesc, nullptr, nullptr, (IDXGISwapChain1**)&swapchain);
	if (FAILED(hr)) {
		std::cout << "Failed to Create SwapChain\n";
		return hr;
	}

	/*-----Create RenderTargetView ... Create Rendering Target details-----*/
	//Describe DescriptorHeap
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	//Create Descriptor Heap
	hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeap));
	if (FAILED(hr)) {
		std::cout << "Failed to Create Descriptor Heap\n";
		return 0;
	}
	//Create RenderTargetView
	D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (int i = 0; i < swapchainDesc.BufferCount; i++) {
		hr = swapchain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i]));
		device->CreateRenderTargetView(backBuffers[i], nullptr, handle);
		handle.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	/*-----Create Fence ... this is used for synchronous process GPU and CPU-----*/
	hr = device->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	if (FAILED(hr)) {
		std::cout << "Failed to Create Fence\n";
		return hr;
	}

	/*-----Create Depth/StencilBuffer ... use for exclude draw hidden surface of Objects-----*/
	//Depth/StencilBufferの定義
	D3D12_RESOURCE_DESC depthResDesc = {};
	depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResDesc.Width = window_Width;
	depthResDesc.Height = window_Height;
	depthResDesc.DepthOrArraySize = 1;
	depthResDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthResDesc.SampleDesc.Count = 1;
	depthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	//Depth/StencilBuffer HeapProperty
	D3D12_HEAP_PROPERTIES depthHeapProp = {};
	depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	depthHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	depthHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	//ClearValue
	D3D12_CLEAR_VALUE depthClearValue = {};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	//Create Depth/StencilBuffer
	hr = device->CreateCommittedResource(&depthHeapProp, D3D12_HEAP_FLAG_NONE, &depthResDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue, IID_PPV_ARGS(&depthBuffer));
	if (FAILED(hr)) {
		std::cout << "Failed to Create DepthStencilBuffer\n";
		return hr;
	}

	/*-----Create Depth/StencilView-----*/
	//describe Depth/StencilView
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hr = device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap));
	if (FAILED(hr)) {
		std::cout << "Failed to Create dsvHeap\n";
		return hr;
	}
	//Create Depth/StencilView
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	device->CreateDepthStencilView(depthBuffer, &dsvDesc, dsvHeap->GetCPUDescriptorHandleForHeapStart());

	return S_OK;
}

/*
Function name is "CreateResources" because of I was also using this function to Create Object Resources, but now is not.
this function used for only Create ConstantBuffer
*/
HRESULT DirectXController::CreateResources(Camera &camera) {
	HRESULT hr;

#ifdef _DEBUG
	EnableDebugLayer();
#endif
	/*-----Create ConstantBuffer-----*/
	//Create ViewMatrix
	DirectX::XMFLOAT3 eye(0, 0, 0); // = camera pos
	DirectX::XMFLOAT3 target(0, 0, eye.z + 1); // = camera Look at
	DirectX::XMFLOAT3 up(0, 1, 0); // = use for compute axis from eye and target
	camera.InitCamera(eye, target, up); //ViewMatrix Mange Camera Class
	//Create ProjectionMatrix
	XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, static_cast<float>(window_Width) / static_cast<float>(window_Height), 1.0f, 10000.0f);
	//describe Resource
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = sizeof(MatrixData) * MAX_OBJECT_COUNT;
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count = 1;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//HeapProperty
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
	//Create ConstantBuffer
	/*
	Map can get Buffer data pointer.
	we can use this variable to update Buffer data
	*/
	hr = constBuffer->Map(0, nullptr, (void**)&mapMatrix);
	if (FAILED(hr)) {
		std::cout << "Failed to Map constBuffer\n";
		return hr;
	}
	for (int i = 0; i < MAX_OBJECT_COUNT; i++) {
		(mapMatrix + i)->v = camera.viewMatrix;
		(mapMatrix + i)->p = projectionMatrix;
		(mapMatrix + i)->eye = eye;
	}
	
	/*-----Create ConstanBufferView-----*/
	//describe DescriptorHeap
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NodeMask = 0;
	descHeapDesc.NumDescriptors = MAX_OBJECT_COUNT;
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	//Create DescriptorHeap
	hr = device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&cbvHeap));
	if (FAILED(hr)) {
		std::cout << "Failed to Create descHeapDesc\n";
		return hr;
	}
	//Get DescriptorHeap Handle where its start
	auto handleOffset = cbvHeap->GetCPUDescriptorHandleForHeapStart();
	//describe ConstantBufferView
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = constBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = sizeof(MatrixData);
	//Create ConstantBufferView
	for (int i = 0; i < MAX_OBJECT_COUNT; i++) {
		device->CreateConstantBufferView(&cbvDesc, handleOffset);
		handleOffset.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		cbvDesc.BufferLocation += sizeof(MatrixData);
	}
	return S_OK;
}

HRESULT DirectXController::CreateShaders() {
	HRESULT hr;
	
	hr = D3DCompileFromFile(L"VertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vertexShader, &errorBlob);
	if (FAILED(hr)) {
		if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			//cannnot find shader File, maybe there is Compiled Shader File
			hr = D3DReadFileToBlob(L"VertexShader.cso", &vertexShader);
			if (FAILED(hr)) {
				std::cout << "Error : There is no VertexShader" << std::endl;
				return hr;
			}
		}
		else {
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";

			std::cout << "Failed to Compile VertexShader\n";
			return hr;
		}
	}

	//PixelShaderのコンパイル
	hr = D3DCompileFromFile(L"PixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pixelShader, &errorBlob);
	if (FAILED(hr)) {
		if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			hr = D3DReadFileToBlob(L"PixelShader.cso", &pixelShader);
			if (FAILED(hr)) {
				std::cout << "Error : There is no PixelShader" << std::endl;
				return hr;
			}
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

	/*-----Create RootSignature ... RootSignature Summarize DescriptorHeaps, and so on(what is rootSignature, descriptor...? ->https://biwanoie.tokyo/ayyvp/ )-----*/
	//describe sampler
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
	//Create Descriptor Table
	D3D12_DESCRIPTOR_RANGE desTbRange[3] = {};
	//DescriptorRange for ConstantBuffer
	desTbRange[0].NumDescriptors = 1;
	desTbRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	desTbRange[0].BaseShaderRegister = 0;
	desTbRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	//DescriptorRange for MaterialBuffer
	desTbRange[1].NumDescriptors = 1;
	desTbRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	desTbRange[1].BaseShaderRegister = 1;
	desTbRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	//DescriptorRange for TextureBuffer
	desTbRange[2].NumDescriptors = 3;
	desTbRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	desTbRange[2].BaseShaderRegister = 0;
	desTbRange[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	//Create RootParameter
	D3D12_ROOT_PARAMETER rootParam[3] = {};
	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParam[0].DescriptorTable.pDescriptorRanges = &desTbRange[0];
	rootParam[0].DescriptorTable.NumDescriptorRanges = 1;

	rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParam[1].DescriptorTable.pDescriptorRanges = &desTbRange[1];
	rootParam[1].DescriptorTable.NumDescriptorRanges = 1;
	
	rootParam[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParam[2].DescriptorTable.pDescriptorRanges = &desTbRange[2];
	rootParam[2].DescriptorTable.NumDescriptorRanges = 1;

	//describe RootSignature
	D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
	rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSigDesc.pParameters = rootParam;
	rootSigDesc.NumParameters = 3;
	rootSigDesc.pStaticSamplers = &samplerDesc;
	rootSigDesc.NumStaticSamplers = 1;
	hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSig, &errorBlob);
	if (FAILED(hr)) {
		std::cout << "Failed to SealizeRootSignature\n";
		return hr;
	}
	//Create RootSignature
	hr = device->CreateRootSignature(0, rootSig->GetBufferPointer(), rootSig->GetBufferSize(), IID_PPV_ARGS(&rootsignature));
	if (FAILED(hr)) {
		std::cout << "Failed to CreateRootSignature\n";
		return hr;
	}

	/*-----Create GraphicsPipeLineState-----*/
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
	hr = device->CreateGraphicsPipelineState(&gPipeLine, IID_PPV_ARGS(&PipeLineState));
	if (FAILED(hr)) {
		std::cout << "Failed to Create GraphicsPipelineState\n";
		return hr;
	}



	return S_OK;
}

HRESULT DirectXController::Draw(Camera& camera) {

	//Set ViewPort
	D3D12_VIEWPORT vp = {};
	vp.Width = window_Width;
	vp.Height = window_Height;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.MaxDepth = 1.0f;
	vp.MinDepth = 0.0f;

	//Set ScisorsRect
	D3D12_RECT sr = {};
	sr.top = 0;
	sr.left = 0;
	sr.right = sr.left + window_Width;
	sr.bottom = sr.top + window_Height;

	/*-----push Commands to ComandQueue-----*/
	//Get BackBuffer to Draw
	auto bbID = swapchain->GetCurrentBackBufferIndex();

	/*
	this Application use Buffer to Draw Image.
	Application has to tell buffer that how buffer is used now.
	*/
	D3D12_RESOURCE_BARRIER BarrierDesc = {};
	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	BarrierDesc.Transition.pResource = backBuffers[bbID];
	BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	cmdList->ResourceBarrier(1, &BarrierDesc);

	cmdList->SetPipelineState(PipeLineState);

	//Set RenderTargetBufferView and Depth/StencilBUfferView
	auto rtvH = rtvHeap->GetCPUDescriptorHandleForHeapStart();
	auto dsvH = dsvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += bbID * device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	cmdList->OMSetRenderTargets(1, &rtvH, false, &dsvH);

	//Clear Depth/StencilBuffer
	cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	//Clear RenderTargetBuffer
	float clearColor[] = { 0.5f,0.5f,0.7f,1.0f };
	cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

	cmdList->RSSetViewports(1, &vp);
	cmdList->RSSetScissorRects(1, &sr);

	cmdList->SetGraphicsRootSignature(rootsignature);

	D3D12_GPU_DESCRIPTOR_HANDLE heapHCBV, heapHMat;

	heapHCBV = cbvHeap->GetGPUDescriptorHandleForHeapStart();

	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	if (LoadedObjCount > 0) {
		for (int objCnt = 0; objCnt < LoadedObjCount; objCnt++) {

			D3D12_GPU_DESCRIPTOR_HANDLE tmpHandle;

			UpdateWorldMatrix(objs[objCnt], objCnt);
			UpdateViewMatrix(camera, objCnt);

			//Switch ConstantBuffer for Each Objects
			cmdList->SetDescriptorHeaps(1, &cbvHeap);
			tmpHandle.ptr = heapHCBV.ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * objCnt;
			cmdList->SetGraphicsRootDescriptorTable(0, tmpHandle);

			cmdList->IASetVertexBuffers(0, 1, &objs[objCnt].vbView);
			cmdList->IASetIndexBuffer(&objs[objCnt].ibView);

			//Switch MaterialBuffer and TextureBuffer for Each Vertices
			heapHMat = objs[objCnt].materialDescHeap->GetGPUDescriptorHandleForHeapStart();
			for (int i = 0; i < objs[objCnt].matRef.size(); i++) {
				//set Texture
				cmdList->SetDescriptorHeaps(1, &objs[objCnt].textureDescHeap); //texture
				D3D12_GPU_DESCRIPTOR_HANDLE heapH = objs[objCnt].textureDescHeap->GetGPUDescriptorHandleForHeapStart();
				heapH.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * (3 * objs[objCnt].matRef[i].matID);
				cmdList->SetGraphicsRootDescriptorTable(2, heapH);
				//set Material
				cmdList->SetDescriptorHeaps(1, &objs[objCnt].materialDescHeap); //material
				tmpHandle.ptr = heapHMat.ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * objs[objCnt].matRef[i].matID;
				cmdList->SetGraphicsRootDescriptorTable(1, tmpHandle);
				cmdList->DrawIndexedInstanced(objs[objCnt].matRef[i].idxNum, 1, objs[objCnt].matRef[i].idxOffset, 0, 0);
			}
		}
	}

	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	cmdList->ResourceBarrier(1, &BarrierDesc);

	//Close CommandList
	cmdList->Close();
	//Execute Command
	ID3D12CommandList* cmdlists[] = { cmdList };
	cmdQueue->ExecuteCommandLists(1, cmdlists);

	//wait for GPU signal which says "Finish Drawing"
	cmdQueue->Signal(fence, ++fenceVal);
	if (fence->GetCompletedValue() != fenceVal) {
		auto event = CreateEvent(nullptr, false, false, nullptr);
		fence->SetEventOnCompletion(fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}

	//Reset Command
	cmdAllocator->Reset();
	cmdList->Reset(cmdAllocator, nullptr);

	//Swap the BackBuffer to Display rendering Image
	swapchain->Present(1, 0);
	
	return S_OK;
}

HRESULT DirectXController::LoadObject(const char* objName) {
	HRESULT hr;
	PathController pc;

	if (LoadedObjCount < MAX_OBJECT_COUNT) { //Check Objects Count
		hr = objs[LoadedObjCount].OBJ_LoadModelData(objName, device);
		if (FAILED(hr)) {
			std::cout << "Cannot Create New Object" << std::endl;
			objs[LoadedObjCount].Release(); //Release Memory
			return E_FAIL;
		}
		char name[256];
		pc.GetLeafDirectryName(objName, name, 256); //ObjectName is [***.obj]
		objs[LoadedObjCount].objName = name;
		LoadedObjCount++; //Increment LoadedObjCount if Object Load is success
	}
	else { //Objects Count is Max
		std::cout << "Cannot Load New Object any more" << std::endl;
		return E_FAIL;
	}

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

HRESULT DirectXController::UpdateWorldMatrix(Object& obj, int objIndex) {
	XMMATRIX posMat = XMMatrixTranslation(obj.transform.position.x, obj.transform.position.y, obj.transform.position.z);
	XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(obj.transform.rotation.x, obj.transform.rotation.y, obj.transform.rotation.z);
	XMMATRIX sizMat = XMMatrixScaling(obj.transform.size.x, obj.transform.size.y, obj.transform.size.z);

	XMMATRIX worldMat = sizMat * rotMat * posMat; //Create New Matrix to hand mapMatrix

	(mapMatrix + objIndex)->w = worldMat;

	return S_OK;
}

HRESULT DirectXController::UpdateViewMatrix(Camera& camera, int objIndex) {
	(mapMatrix + objIndex)->v = camera.viewMatrix; //set Camera's viewMatrix to mapMatrix
	(mapMatrix + objIndex)->eye = camera.pos;

	return S_OK;
}

/*
this function is use for desplay DirectX Error
*/
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
	if (cbvHeap)cbvHeap->Release();

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
	if (constBuffer)constBuffer->Release();
}
