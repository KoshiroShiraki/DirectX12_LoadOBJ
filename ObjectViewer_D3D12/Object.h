#pragma once
#include<DirectXMath.h>
#include<DirectXTex.h>
#include<d3d12.h>
#include<d3dx12.h>
#include<vector>
#include<map>
#include<iostream>
#include<fstream>
#include<time.h>
#include"PathController.h"
#include"Camera.h"
using namespace DirectX;

#define MAX_READ_LINEDATA 10000 //メッシュ/マテリアルファイルを読み込むときの最大文字数/行
#define MAX_MATERIAL_REFERENCE 128 

#pragma comment(lib,"DirectXTex.lib")

struct SimpleVertex {
	XMFLOAT3 pos;
	XMFLOAT2 uv;
};

struct ObjTransrom {
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	XMFLOAT3 size;
};

//シェーダリソースとして使われる(頂点)
struct OBJVertex {
	XMFLOAT3 pos;
	XMFLOAT3 normal;
	XMFLOAT2 uv;
};

//マテリアル参照情報
struct OBJMaterialRef {
	std::string matName;

	int matID;

	int idxOffset;
	int idxNum;

	OBJMaterialRef();
};

//シェーダリソースとして使われる(マテリアル)
struct OBJMaterialCB {
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;
	float Nspecular;

	char padding[204];
};

//マテリアル情報
struct OBJMaterial {
	OBJMaterialCB mcb;

	std::string materialName;

	int texCnt = 0;

	char ambTexPath[MAX_PATH_LENGTH];
	char difTexPath[MAX_PATH_LENGTH];
	char speTexPath[MAX_PATH_LENGTH];

	OBJMaterial();
	void Init();
};

//面情報の解析に使う
struct OBJFaceData {
	int vertexCnt;
	int** dataIndex;

	OBJFaceData(int cnt);

	~OBJFaceData();

	OBJFaceData(const OBJFaceData& fd);
};

//画像データ
struct ImageData {
	size_t rowPitch;
	DXGI_FORMAT format;
};

//テクスチャバッファ
struct OBJTextureBuffers {
	ID3D12Resource* ambTexBuffer = nullptr;
	ID3D12Resource* difTexBuffer = nullptr;
	ID3D12Resource* speTexBuffer = nullptr;

	~OBJTextureBuffers();
};

class Object {
public:
	Object();
	Object(std::string name);
	~Object();

	ObjTransrom transform;

	bool ObjectLoaded = false;	//実体化されたオブジェクトクがすでにモデルをロード済みか

	std::string objName; //オブジェクト名

	std::vector<OBJVertex> vertices;
	std::vector<OBJMaterial> materials;
	std::vector<OBJMaterialRef> matRef;
	std::vector<unsigned> indices;

	int texCount = 0; //テクスチャ数

	/*-----DirectX12描画用-----*/
public:
	ID3D12Resource* vertexBuffer = nullptr;
	ID3D12Resource* indexBuffer = nullptr;
	ID3D12Resource* materialBuffer = nullptr;
	std::vector<OBJTextureBuffers> texBuffers;
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	D3D12_INDEX_BUFFER_VIEW ibView = {};
	ID3D12DescriptorHeap* materialDescHeap = nullptr;
	ID3D12DescriptorHeap* textureDescHeap = nullptr;
	XMMATRIX worldMat; //ワールド行列(ワールド位置はオブジェクトごとに違うので自己管理)
	/*-------------------------*/

	/*-----OBJパース用-----*/
	HRESULT OBJ_LoadModelData(std::string path, ID3D12Device* device);
	HRESULT checkPathLength(size_t length);
	void OBJ_splitBlank(std::string str, std::vector<std::string>& data);
	void OBJ_splitSlash(std::string str, int* data);
	int findMaterialIndex(std::vector<OBJMaterialRef> mr, std::string material);
	HRESULT OBJ_CreateTextureBuffer(const wchar_t* pathName, size_t pathLength, int materialNum, int textureNum, ID3D12Device* device);
	/*---------------------*/

	/*-----3Dオブジェクト用-----*/
	HRESULT CreatePlane();
	HRESULT CreateCube();
	/*--------------------------*/

	/*-----共通-----*/
	HRESULT CreateVertexBuffer();
	HRESULT CreateIndexBuffer();
	HRESULT CreateMaterialBuffer();
	HRESULT CreateTextureBuffer();

	/*-----描画-----*/
	HRESULT DrawObjet(ID3D12GraphicsCommandList* cmdList, ID3D12PipelineState* pipelineState, ID3D12RootSignature* rootsignature, Camera camera, D3D12_CPU_DESCRIPTOR_HANDLE rtvH, D3D12_CPU_DESCRIPTOR_HANDLE dsvH, D3D12_VIEWPORT vp, D3D12_RECT rc);

	template<typename T>
	void vectorRelease(std::vector<T>& vec);
	void Release();
};