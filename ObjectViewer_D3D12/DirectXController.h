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

using namespace DirectX;

struct ShadowMatrixData {
	XMMATRIX w;
	XMMATRIX v_light;
	XMMATRIX p;

	char padding[64];
};

struct MatrixData {
	XMMATRIX w;
	XMMATRIX v_light;
	XMMATRIX v_camera;
	XMMATRIX p_perspective;
	XMMATRIX p_orthographic;
	XMFLOAT3 eye;
	float dammy;
	XMFLOAT3 light_col;

	char padding[164];
};

class DirectXController {
public:
	DirectXController();
	~DirectXController();

private:
	UINT m_fenceVal = 0;
	static const int RT_BUFFER_COUNT = 2; //バッファリング数
	static const int RT_MULTIPASS_COUNT = 1; //マルチパスレンダリングのパス数
	static const int MAX_OBJECT_COUNT = 50;

	/*-----DirectXインターフェース-----*/

public:
	ID3D12Device* m_device = nullptr;

	IDXGIFactory6* m_factory = nullptr;

	IDXGISwapChain4* m_swapchain = nullptr;

	ID3D12CommandAllocator* m_cmdAllocator = nullptr;
	ID3D12GraphicsCommandList* m_cmdList = nullptr;
	ID3D12CommandQueue* m_cmdQueue = nullptr;

	//シャドウマップ(ライト視点レンダリング) m_s〇〇
	ID3D12Resource* m_shadowBuffer = nullptr; //深度/ステンシルバッファ or シェーダリソース(テクスチャ)バッファ
	ID3D12DescriptorHeap* m_sdsvHeap = nullptr; //ディスクリプタヒープ(深度/ステンシルバッファ用)
	ID3D12DescriptorHeap* m_ssrvHeap = nullptr; //ディスクリプタヒープ(シェーダリソース(テクスチャ)用)
	ID3D12Resource* m_sconstBuffer = nullptr; //定数バッファ
	ID3D12DescriptorHeap* m_scbvHeap = nullptr; //ディスクリプタヒープ(定数バッファ用)
	ShadowMatrixData* m_smapMatrix; //シャドウマップ用定数バッファのマップ用変数
	ID3D12PipelineState* m_spipeLineState = nullptr; //パイプラインステート
	ID3DBlob* m_srootSig = nullptr; //ルートシグネチャのバイナリデータオブジェクト
	ID3D12RootSignature* m_srootSignature = nullptr; //ルートシグネチャ
	DX12Shader m_svertexShader; //バーテックスシェーダ
	const int m_shadowSolution = 4096;


	//通常レンダリング(カメラ視点レンダリング) m_m〇〇
	ID3D12Resource* m_mrenderBuffers[RT_MULTIPASS_COUNT]; //レンダーターゲットバッファ or シェーダリソース(テクスチャ)バッファ
	ID3D12DescriptorHeap* m_mrtvHeap = nullptr; //ディスクリプタヒープ(レンダーターゲット用)
	ID3D12DescriptorHeap* m_msrvHeap = nullptr; //ディスクリプタヒープ(シェーダリソース用)
	ID3D12Resource* m_mdepthBuffer = nullptr; //深度/ステンシルバッファバッファ
	ID3D12DescriptorHeap* m_mdsvHeap = nullptr; //ディスクリプタヒープ(深度/ステンシルバッファ用)
	ID3D12Resource* m_mconstBuffer = nullptr; //定数バッファ
	ID3D12DescriptorHeap* m_mcbvHeap = nullptr; //ディスクリプタヒープ(定数バッファ用)
	MatrixData* m_mmapMatrix; //定数バッファのマップ用変数
	ID3D12PipelineState* m_mpipeLineState = nullptr; //パイプラインステート
	ID3DBlob* m_mrootSig = nullptr; //ルートシグネチャのバイナリデータオブジェクト
	ID3D12RootSignature* m_mrootsignature = nullptr; //ルートシグネチャ
	DX12Shader m_mvertexShader; //バーテックスシェーダ
	DX12Shader m_mpixelShader; //ピクセルシェーダ

	//最終レンダリング m_f〇〇
	ID3D12Resource* m_backBuffers[RT_BUFFER_COUNT]; //バックバッファ(レンダーターゲットバッファ)
	ID3D12DescriptorHeap* m_frtvHeap = nullptr; //ディスクリプタヒープ(レンダーターゲット用)
	ID3D12PipelineState* m_fpipeLineState = nullptr; //パイプラインステート
	ID3DBlob* m_frootSig = nullptr; //ルートシグネチャのバイナリデータオブジェクト
	ID3D12RootSignature* m_frootsignature = nullptr; //ルートシグネチャ
	DX12Shader m_fvertexShader; //バーテックスシェーダ
	DX12Shader m_fpixelShader; //ピクセルシェーダ
	ID3D12Resource* m_frenderPolygon = nullptr; //バーテックスバッファ(m_mrederBuffersの出力結果を張り付けるポリゴン用)
	D3D12_VERTEX_BUFFER_VIEW m_frenderVbv; //バーテックスバッファビュー

	ID3D12Resource* m_floorPolygon = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_floorPolygonVbv;

	ID3D12Fence* m_fence = nullptr;

	ID3DBlob* m_errorBlob = nullptr;
	/*--------------------------*/

	std::vector<DX12ObjectFormatOBJ*> m_objsOBJ; //描画する3Dモデル
	int m_LoadedObjCount = 0; //描画済みモデル数

private:
	D3D12_VIEWPORT m_fvp = {};
	D3D12_VIEWPORT m_svp = {};
	D3D12_RECT m_fsr = {};

public:
	HRESULT InitD3D(HWND hwnd);
	HRESULT CreateRenderResources();
	HRESULT CreateBackBuffers();
	HRESULT CreateRenderBuffers();
	HRESULT CreateDepthStencilBuffer();
	HRESULT CreateShadowBuffer();
	HRESULT CreateFinalRenderPolygon();
	HRESULT CreateConstBuffers(Camera& camera, Light& light);
	HRESULT CreateShaders();
	HRESULT CreateShadowMapGraphicsPipeLine();
	HRESULT CreateFinalGraphicsPipeLine();
	HRESULT SetGraphicsPipeLine();
	HRESULT DrawFromLight(Light& light);
	HRESULT DrawFromCamera(Camera camera, Light light);
	HRESULT finalDraw();
	HRESULT UpdateObjTransform(HWND hwnd[9], int offset, XMFLOAT3& objData);
	HRESULT UpdateObjTransform(HWND hwnd[9], float value[3], int offset, XMFLOAT3& objData);
	HRESULT UpdateWorldMatrix(Object& obj, int objIndex);
	HRESULT UpdateViewMatrix(Camera camera, Light light, int objIndex);
	HRESULT LoadObject(const char* objName);
	HRESULT DeleteObject(int objIdx);
	
	void EnableDebugLayer();
	void Release();
};