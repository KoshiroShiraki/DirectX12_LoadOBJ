/*
☆DirectX管理クラス
*/

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

#define MAX_OBJECT_COUNT 20 //最大オブジェクト数

//定数バッファ用
struct MatrixData {
	DirectX::XMMATRIX w;
	DirectX::XMMATRIX v;
	DirectX::XMMATRIX p;
	DirectX::XMFLOAT3 eye;

	char padding[52];
};

class DirectXController {
public:
	/*-----メンバ変数-----*/
	ID3D12Device* device = nullptr;

	IDXGIFactory6* factory = nullptr;

	IDXGISwapChain4* swapchain = nullptr;

	ID3D12CommandAllocator* cmdAllocator = nullptr;
	ID3D12GraphicsCommandList* cmdList = nullptr;
	ID3D12CommandQueue* cmdQueue = nullptr;

	ID3D12DescriptorHeap* rtvHeap = nullptr;
	ID3D12DescriptorHeap* dsvHeap = nullptr;
	ID3D12DescriptorHeap* basicDescHeap = nullptr;

	ID3DBlob* vertexShader = nullptr;
	ID3DBlob* pixelShader = nullptr;
	ID3DBlob* errorBlob = nullptr;
	ID3DBlob* rootSig = nullptr;

	ID3D12PipelineState* PipeLineState = nullptr;

	ID3D12RootSignature* rootsignature = nullptr;

	ID3D12Fence* fence = nullptr;
	UINT fenceVal = 0;

	MatrixData* mapMatrix;

	float angle = 0;

	std::vector<ID3D12Resource*> backBuffers;
	ID3D12Resource* depthBuffer = nullptr;
	//ID3D12Resource* vertexBuffer = nullptr;
	//ID3D12Resource* indexBuffer = nullptr;
	ID3D12Resource* constBuffer = nullptr;

	Object objs[MAX_OBJECT_COUNT];
	int LoadedObjCount = 0;

	XMMATRIX worldMatrix;
	XMMATRIX viewMatrix;
	XMMATRIX projectionMatrix;

	/*-----コンストラクタ/デストラクタ-----*/
	DirectXController();
	~DirectXController();

	/*-----メンバ関数-----*/
	HRESULT InitD3D(HWND hwnd);
	HRESULT CreateResources(Camera &camera);
	HRESULT CreateShaders();
	HRESULT SetGraphicsPipeLine();
	HRESULT Draw();
	HRESULT UpdateObjTransform(HWND hwnd[9], int offset, XMFLOAT3& objData);
	HRESULT UpdateWorldMatrix(Object &obj);
	HRESULT UpdateViewMatrix(Camera &camera);
	HRESULT LoadObject(const char* objName);
	void EnableDebugLayer();
	void Release();

};