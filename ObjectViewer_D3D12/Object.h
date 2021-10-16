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
#include<DirectXTex.h>
#include<d3d12.h>
#include<d3dx12.h>
#include<vector>
#include<map>
#include<iostream>
#include<fstream>
#include<time.h>
#include"ConstValue.h"
#include"PathController.h"
using namespace DirectX;

#define MAX_READ_LINEDATA 1024 //メッシュ/マテリアルファイルを読み込むときの最大文字数/行
#define MAX_MATERIAL_REFERENCE 128 

#pragma comment(lib,"DirectXTex.lib")

/*-----オブジェクト構造体-----*/
struct ObjTransrom {
	XMFLOAT3 position; //位置
	XMFLOAT3 size; //大きさ
	XMFLOAT3 rotation; //回転
};

/*-----obj用構造体-----*/
//頂点
struct OBJVertex {
	XMFLOAT3 pos; //頂点座標
	XMFLOAT3 normal; //法線ベクトル
	XMFLOAT2 uv; //UV座標
};
//マテリアル参照データ。GPUにdraw命令を送る際にこの構造体データをもとにマテリアルを切り替える
struct OBJMaterialRef {
	std::string matName; //マテリアル名

	int matID; //マテリアルID

	int idxOffset; //マテリアル適用開始位置
	int idxNum;; //マテリアル適用数

	OBJMaterialRef();
};
//コンスタントバッファとしてシェーダに送るマテリアルデータ
struct OBJMaterialCB {
	XMFLOAT4 ambient; //環境反射色
	XMFLOAT4 diffuse; //拡散反射色
	XMFLOAT4 specular; //鏡面反射色
	float Nspecular; //鏡面反射指数

	char padding[204];
};
//マテリアル
struct OBJMaterial {
	OBJMaterialCB mcb; //シェーダに送るデータ

	std::string materialName;

	char ambTexPath[MAX_PATH_LENGTH]; //アンビエントテクスチャパス
	char difTexPath[MAX_PATH_LENGTH]; //ディフューズテクスチャパス
	char speTexPath[MAX_PATH_LENGTH]; //スペキュラテクスチャパス

	OBJMaterial();
	void Init();
};

struct OBJFaceData {
	int vertexCnt; //面を構成する頂点数
	int** dataIndex; //各データのインデックス

	OBJFaceData(int cnt);

	~OBJFaceData();

	OBJFaceData(const OBJFaceData& fd);
};

class Object {
public:
	/*-----メンバ変数-----*/
	ObjTransrom transform; //オブジェクト位置姿勢大きさ

	bool ObjectLoaded = false; //オブジェクトがロードされているかチェックする(多重ロードを防ぐ)

	std::string objName; //オブジェクト名

	std::vector<OBJVertex> vertices; //.obj頂点データ
	std::vector<OBJMaterial> materials; //マテリアル
	std::vector<OBJMaterialRef> matRef; //マテリアル参照
	std::vector<unsigned> indices; //インデックス

	/*-----DirectXに必要なデータ-----*/
	ID3D12Resource* vertexBuffer = nullptr; //頂点バッファ
	ID3D12Resource* indexBuffer = nullptr; //インデックスバッファ
	ID3D12Resource* materialBuffer = nullptr; //マテリアルバッファ
	D3D12_VERTEX_BUFFER_VIEW vbView = {}; //自身の持つバッファのビュー
	D3D12_INDEX_BUFFER_VIEW ibView = {}; //自身の持つバッファのビュー
	ID3D12DescriptorHeap* materialDescHeap = nullptr; //マテリアル用ディスクリプタヒープ
	/*-------------------------------*/


	/*-----コンストラクタ/デストラクタ-----*/
	Object();
	Object(std::string name);
	~Object();

	/*-----メンバ関数-----*/
	HRESULT OBJ_LoadModelData(std::string path, ID3D12Device* device); //OBJモデル読み込み
	/*
	以下の2つに関しては、膨大に繰り返される処理であるため極力if分岐処理を減らすために別々の関数にする
	*/

	HRESULT checkPathLength(size_t length); //パスの長さ検証。エラーなら1、それ以外は0
	void OBJ_splitBlank(std::string str, std::vector<std::string>& data); //空白で分割, dataは分割後のデータ格納用配列(可変長にしたのは配列サイズが一定にならないから)
	void OBJ_splitSlash(std::string str, int* data); //スラッシュで分割 dataは分割後のデータ格納用配列(ポインタ渡しなのは固定長配列だから)
	int findMaterialIndex(std::vector<OBJMaterialRef> mr, std::string material);
	template<typename T>
	void vectorRelease(std::vector<T>& vec); //vector解放関数
	void Release(); //インスタンス解放関数
};