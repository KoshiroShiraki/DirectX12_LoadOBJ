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
using namespace DirectX;

#define MAX_READ_LINEDATA 10000 //メッシュ/マテリアルファイルを読み込むときの最大文字数/行
#define MAX_MATERIAL_REFERENCE 128 

#pragma comment(lib,"DirectXTex.lib")

/*
this programu is load "wavefront OBJ(.obj)".
file format reference -> https://ja.wikipedia.org/wiki/Wavefront_.obj%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB (Caution : this is not official)
*/

struct ObjTransrom {
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	XMFLOAT3 size;
};

//use for shader resources
struct OBJVertex {
	XMFLOAT3 pos;
	XMFLOAT3 normal;
	XMFLOAT2 uv;
};

//use for change Descriptor in Draw function
struct OBJMaterialRef {
	std::string matName;

	int matID;

	int idxOffset; //material start Posiiton
	int idxNum; //Apllying material Count

	OBJMaterialRef();
};

//use for shader resources
struct OBJMaterialCB {
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;
	float Nspecular;

	char padding[204];
};

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

//use for Analyze .obj file's FaceData
struct OBJFaceData {
	int vertexCnt;
	int** dataIndex;

	OBJFaceData(int cnt);

	~OBJFaceData();

	OBJFaceData(const OBJFaceData& fd);
};
struct ImageData {
	size_t rowPitch;
	DXGI_FORMAT format;
};

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

	bool ObjectLoaded = false;	//if this classObject already loaded ObjectData, never load more ObjectData

	std::string objName;

	/*
	these vector used to get data from .obj file.
	and these vector also used to create Buffers.
	*/
	std::vector<OBJVertex> vertices;
	std::vector<OBJMaterial> materials;
	std::vector<OBJMaterialRef> matRef;
	std::vector<unsigned> indices;

	int texCount = 0; //Count of Texture

	/*-----Use for DirectX Drawing-----*/
	ID3D12Resource* vertexBuffer = nullptr;
	ID3D12Resource* indexBuffer = nullptr;
	ID3D12Resource* materialBuffer = nullptr;
	std::vector<OBJTextureBuffers> texBuffers;
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	D3D12_INDEX_BUFFER_VIEW ibView = {};
	ID3D12DescriptorHeap* materialDescHeap = nullptr;
	ID3D12DescriptorHeap* textureDescHeap = nullptr;
	/*--------------------------------*/

	HRESULT OBJ_LoadModelData(std::string path, ID3D12Device* device);
	HRESULT checkPathLength(size_t length);
	void OBJ_splitBlank(std::string str, std::vector<std::string>& data);
	void OBJ_splitSlash(std::string str, int* data);
	int findMaterialIndex(std::vector<OBJMaterialRef> mr, std::string material);
	HRESULT OBJ_CreateTextureBuffer(const wchar_t* pathName, size_t pathLength, int materialNum, int textureNum, ID3D12Device* device);
	template<typename T>
	void vectorRelease(std::vector<T>& vec);
	void Release();
};