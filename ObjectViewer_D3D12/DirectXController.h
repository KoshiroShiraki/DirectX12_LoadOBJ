#pragma once
#include<Windows.h>
#include<d3d12.h>
#include<d3dx12.h>
#include<dxgi1_6.h>
#include<d3dcompiler.h>
#include<DirectXMath.h>
#include<vector>
#include<string>
#include"ConstValue.h"
#include"Object.h"
#include"Camera.h"
#include"PathController.h"

#ifdef _DEBUG
#include<iostream>
#endif

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"DirectXTex.lib")
#pragma warning(disable: 4996)

#define MAX_OBJECT_COUNT 10

struct MatrixData {
	DirectX::XMMATRIX w;
	DirectX::XMMATRIX v;
	DirectX::XMMATRIX p;
	DirectX::XMFLOAT3 eye;

	char padding[52];
};
struct testTexture {
	unsigned char R, B, G, A;
};

class DirectXController {
public:
	DirectXController();
	~DirectXController();

	/*-----DirectX Interface-----*/
	ID3D12Device* device = nullptr;

	IDXGIFactory6* factory = nullptr;

	IDXGISwapChain4* swapchain = nullptr;

	ID3D12CommandAllocator* cmdAllocator = nullptr;
	ID3D12GraphicsCommandList* cmdList = nullptr;
	ID3D12CommandQueue* cmdQueue = nullptr;

	ID3D12DescriptorHeap* rtvHeap = nullptr;
	ID3D12DescriptorHeap* dsvHeap = nullptr;
	ID3D12DescriptorHeap* cbvHeap = nullptr;

	ID3DBlob* vertexShader = nullptr;
	ID3DBlob* pixelShader = nullptr;
	ID3DBlob* errorBlob = nullptr;
	ID3DBlob* rootSig = nullptr;

	ID3D12PipelineState* PipeLineState = nullptr;

	ID3D12RootSignature* rootsignature = nullptr;

	std::vector<ID3D12Resource*> backBuffers;
	ID3D12Resource* depthBuffer = nullptr;
	ID3D12Resource* constBuffer = nullptr;

	ID3D12Fence* fence = nullptr;
	UINT fenceVal = 0;
	/*--------------------------*/

	MatrixData* mapMatrix; //use for Buffer Map(update ConstantBuffer Value)

	std::vector<Object> objs; //Array of Objects
	int LoadedObjCount = 0;

	HRESULT InitD3D(HWND hwnd);	//hwnd = MainWindowHandle
	HRESULT CreateResources(Camera& camera); //camera = camera
	HRESULT CreateShaders();
	HRESULT SetGraphicsPipeLine();
	HRESULT Draw(Camera& camera);	//camera = camera
	HRESULT UpdateObjTransform(HWND hwnd[9], int offset, XMFLOAT3& objData);	//hwnd[9] = EditBox used for Object transform, offset = arrays offset, objData = ObjectTransform data
	HRESULT UpdateWorldMatrix(Object& obj, int objIndex);	//obj = Object which you want to update, objIndex = obj's index
	HRESULT UpdateViewMatrix(Camera &camera, int objIndex);	//camera = Camera which you want to update, objIndex = obj's index
	HRESULT LoadObject(const char* objName); //objName = Object Name which you want to load
	
	void EnableDebugLayer();
	void Release();	//safe Release function for DIrectX Interfaces
};