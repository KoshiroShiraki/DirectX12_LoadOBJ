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
#include"Light.h"
#include"PathController.h"
#include"DX12Shader.h"
#include"DX12Object3D.h"
#include"ErrorDebug.h"
#include"DX12ObjectFormatOBJ.h"

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

class DirectXController {
public:
	DirectXController();
	~DirectXController();

	/*-----変数-----*/
private:
	UINT m_fenceVal = 0;
	static const int RT_BUFFER_COUNT = 2; //バッファリング数
	static const int RT_MULTIPASS_COUNT = 1; //マルチパスレンダリングのパス数

	/*-----DirectXインターフェース-----*/
public:
	ID3D12Device* m_device = nullptr;

	IDXGIFactory6* m_factory = nullptr;

	IDXGISwapChain4* m_swapchain = nullptr;

	ID3D12CommandAllocator* m_cmdAllocator = nullptr;
	ID3D12GraphicsCommandList* m_cmdList = nullptr;
	ID3D12CommandQueue* m_cmdQueue = nullptr;

	ID3D12DescriptorHeap* m_rtvHeap = nullptr; //最終出力結果を書き込むレンダーターゲットビューヒープ
	ID3D12DescriptorHeap* m_mrtvHeap = nullptr; //マルチパスレンダリングにおける出力結果を書き込むレンダーターゲットビューヒープ
	ID3D12DescriptorHeap* m_msrvHeap = nullptr; //マルチパスレンダリングにおける出力結果をテクスチャとして扱うためのシェーダーリソースビューヒープ
	ID3D12DescriptorHeap* m_dsvHeap = nullptr;
	ID3D12DescriptorHeap* m_sddvHeap = nullptr; //shadow Depth heap
	ID3D12DescriptorHeap* m_sdtvHeap = nullptr; //shadow Texture heap
	ID3D12DescriptorHeap* m_cbvHeap = nullptr;
	ID3D12DescriptorHeap* m_scbvHeap = nullptr; //シャドウマップ用定数バッファディスクリプタヒープ

	//ID3DBlob* m_vertexShader = nullptr;
	//ID3DBlob* m_pixelShader = nullptr;
	//ID3DBlob* m_fvertexShader = nullptr;
	//ID3DBlob* m_fpixelShader = nullptr;
	ID3DBlob* m_errorBlob = nullptr;
	ID3DBlob* m_rootSig = nullptr;
	ID3DBlob* m_frootSig = nullptr;
	DX12Shader m_vertexShader;
	DX12Shader m_pixelShader;
	DX12Shader m_fvertexShader;
	DX12Shader m_fpixelShader;

	ID3D12PipelineState* m_PipeLineState = nullptr;
	ID3D12PipelineState* m_fPipeLineState = nullptr;
	ID3D12PipelineState* m_smPipeLineState = nullptr; //シャドウマップ用パイプライン
	ID3D12PipelineState* m_pePipeLineState = nullptr;; //ポストエフェクト用パイプライン

	ID3D12RootSignature* m_rootsignature = nullptr;
	ID3D12RootSignature* m_frootsignature = nullptr;

	ID3D12Resource* m_backBuffers[RT_BUFFER_COUNT]; //バックバッファ
	ID3D12Resource* m_depthBuffer = nullptr; //深度バッファ
	ID3D12Resource* m_constBuffer = nullptr; //定数バッファ
	ID3D12Resource* m_shadowBuffer = nullptr; //シャドウマップバッファ(≒深度バッファ)
	ID3D12Resource* m_shadowConstBuffer = nullptr; //シャドウマップ用定数バッファ
	ID3D12Resource* m_renderBuffers[RT_MULTIPASS_COUNT]; //1パス目のレンダリング結果書き込みバッファ(テクスチャとして使う)

	ID3D12Resource* m_finalRenderPolygon = nullptr; //最終レンダリング結果表示ポリゴン。出来上がったイメージテクスチャをこのポリゴンに張り付ける
	D3D12_VERTEX_BUFFER_VIEW m_finarRenderVbv;
	ID3D12Resource* m_floorPolygon = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_floorPolygonVbv;

	ID3D12Fence* m_fence = nullptr;
	/*--------------------------*/

public:
	MatrixData* m_mapMatrix; //定数バッファのマップ用変数
	MatrixData* m_shadowMapMatrix; //シャドウマップ用定数バッファのマップ用変数

	//std::vector<Object> m_objs; //Array of Objects
	//DX12Object3D* testOBJ;
	//std::vector<DX12Object3D> testOBJs;
	std::vector<DX12ObjectFormatOBJ*> m_objsOBJ;
	int m_LoadedObjCount = 0;

private:
	D3D12_VIEWPORT m_vp = {};
	D3D12_RECT m_sr = {};

public:
	HRESULT InitD3D(HWND hwnd);	//hwnd = MainWindowHandle
	HRESULT CreateRenderResources(); //レンダリングバッファの生成(表示用、シャドウマップ用、ポストエフェクト用)
	HRESULT CreateBackBuffers(); //最終的な描画結果を画面に出力するバッファ(マルチパス用)
	HRESULT CreateRenderBuffers(); //途中の描画結果を出力するバッファ
	HRESULT CreateDepthStencilBuffer(); //デプスステンシルバッファの生成
	HRESULT CreateFinalRenderPolygon(); //査収レンダリング結果表示ポリゴンを生成する
	HRESULT CreateFloorPolygon();
	HRESULT CreateConstBuffers(Camera& camera, Light& light); //camera = camera
	HRESULT CreateShaders();
	HRESULT CreateShadowMapGraphicsPipeLine();
	HRESULT CreateForwardGraphicsPipeLine();
	HRESULT CreateFinalGraphicsPipeLine();
	HRESULT SetGraphicsPipeLine();
	HRESULT DrawFromCamera(Camera& camera);	//camera = camera
	HRESULT DrawFromLight(Light& light);
	HRESULT finalDraw();
	HRESULT UpdateObjTransform(HWND hwnd[9], int offset, XMFLOAT3& objData);	//hwnd[9] = EditBox used for Object transform, offset = arrays offset, objData = ObjectTransform data
	HRESULT UpdateObjTransform(HWND hwnd[9], float value[3], int offset, XMFLOAT3& objData); //use for scroll bar
	HRESULT UpdateWorldMatrix(Object& obj, int objIndex);	//obj = Object which you want to update, objIndex = obj's index
	HRESULT UpdateViewMatrix(Camera &camera, int objIndex);	//camera = Camera which you want to update, objIndex = obj's index
	HRESULT LoadObject(const char* objName); //objName = Object Name which you want to load
	HRESULT DeleteObject(int objIdx);
	
	void EnableDebugLayer();
	void Release();	//safe Release function for DIrectX Interfaces
};