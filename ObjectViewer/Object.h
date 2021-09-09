/*
☆オブジェクト管理クラス

設計方針
・ID3D12ResourceはObjectクラスで管理
・DirectXControllerクラスで単一のViewを管理し、描画のたびにBufferLocationとWidthを書き換える

PMDモデル読み込みは以下の書籍を模倣
川野 竜一.DirectX 12の魔導書 3Dレンダリングの基礎からMMDモデルを踊らせるまで.株式会社翔泳社.Kindle 版.
*/

#pragma once
#include<DirectXMath.h>
#include<d3d12.h>
#include<d3dx12.h>
#include<vector>
#include<map>
#include<iostream>
#include<fstream>
#include<time.h>
#include"ConstValue.h"
using namespace DirectX;

/*-----オブジェクト用構造体-----*/
struct vertex {
	XMFLOAT3 pos;
	XMFLOAT2 uv;
};

/*-----PMD用構造体-----*/
struct PMDHeader {
	float version;
	char model_name[20];
	char comment[256];
};
struct PMDVertex {
	XMFLOAT3 pos;
	XMFLOAT3 normal;
	XMFLOAT2 uv;
	unsigned short boneNo[2];
	unsigned char boneWeight;
	unsigned char edgeFlg;
};
#pragma pack(1) //自動的なパディングを防ぐ
struct PMDMaterial {
	XMFLOAT3 diffuse;
	float alpha;
	float specularity;
	XMFLOAT3 specular;
	XMFLOAT3 ambient;
	unsigned char tonnIdx;
	unsigned char edgeFlg;
	unsigned int indicesNum;
	char texFilePath[20];
};
#pragma pack()
struct MaterialForHlsl {
	XMFLOAT3 diffuse;
	float alpha;
	XMFLOAT3 specular;
	float specularity;
	XMFLOAT3 ambient;
};
struct AdditionalMaterial {
	std::string texpath;
	int toonIdx;
	bool edgeFlg;
};
struct Material {
	unsigned int indicesNum;
	MaterialForHlsl material;
	AdditionalMaterial additional;

};
#pragma pack(1)
struct PMDBone {
	char boneName[20];
	unsigned short parentNo;
	unsigned short nextNo;
	unsigned char type;
	unsigned short ikBoneNo;
	XMFLOAT3 pos;
};
#pragma pack()

/*-----obj用構造体-----*/
struct OBJVertex {
	XMFLOAT3 pos;
	XMFLOAT3 normal;
	XMFLOAT2 uv;
	/*PMDVertexとサイズを合わせるためのダミー(InputLayoutを共有するため)*/
	unsigned short dammy1;
	unsigned char dammy2;
	unsigned char dammy3;
};

struct OBJFaceInfo {
	std::string fi[4];
	//面情報が3つのとき,f[3] = nullptr;
};

class Object {
public:
	/*-----メンバ変数-----*/
	bool ObjectLoaded = false; //オブジェクトロード系の関数は必ずこの値を参照、例えばPMDオブジェクトを読み込んだ後にOBJオブジェクトを読み込むようなことは絶対にしない

	std::vector<unsigned char> PMDvertices; //頂点(PMD用)
	std::vector<OBJVertex> OBJvertices;
	std::vector<unsigned> indices; //インデックス

	ID3D12Resource* vertexBuffer = nullptr; //頂点バッファ
	ID3D12Resource* indexBuffer = nullptr; //インデックスバッファ
	ID3D12Resource* materialBuffer = nullptr; //マテリアルバッファ
	D3D12_VERTEX_BUFFER_VIEW vbView = {}; //自身の持つバッファのビュー
	D3D12_INDEX_BUFFER_VIEW ibView = {}; //自身の持つバッファのビュー

	D3D12_INPUT_ELEMENT_DESC *inputLayout; //インプットレイアウト(ファイルフォーマットに応じて変わるので、オブジェクト側で管理)

	/*-----コンストラクタ/デストラクタ-----*/
	Object();
	~Object();

	/*-----メンバ関数-----*/
	HRESULT LoadPMDData(std::string ModelName, ID3D12Device* device); //PMDモデル読み込み
	HRESULT LoadOBJData(std::string ModelName, ID3D12Device* device); //OBJモデル読み込み
	std::vector<std::string> split(std::string str, std::string splitter); //文字列分割(分割する文字列, 分割ワード)
	template<typename T>
	void vectorRelease(std::vector<T>& vec); //vector解放関数
	void Release(); //インスタンス解放関数
};