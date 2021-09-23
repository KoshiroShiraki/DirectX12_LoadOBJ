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

/*-----obj用構造体-----*/
//頂点
struct OBJVertex {
	XMFLOAT3 pos; //頂点座標
	XMFLOAT3 normal; //法線ベクトル
	XMFLOAT2 uv; //UV座標
};
//マテリアル
struct OBJMaterial {
	std::string materialName; //マテリアル名
	XMFLOAT3 ambient; //環境反射色
	XMFLOAT3 diffuse; //拡散反射色
	XMFLOAT3 specular; //鏡面反射色
	float Nsplecelar; //鏡面反射指数
	std::string TexturePath; //テクスチャパス
};
//↑wikipediaによれば、ほかにも多くの項目があるが、今回は上の6つを使用する


struct OBJFaceInfo {
	std::vector<std::string> fi; //面格納用文字列
	int faceNum; //面を構成する頂点数
};

struct OBJFaceData {
	int faceNum; //頂点数
	std::vector<int[3]> dataID; //頂点ID。基本的に v / vt / vn だが、モデルデータによってはvtとvnがない場合があるので、その時は0を代入する
};

class Object {
public:
	/*-----メンバ変数-----*/
	bool ObjectLoaded = false; //オブジェクトがロードされているかチェックする(多重ロードを防ぐ)

	std::string objName; //オブジェクト名

	std::vector<OBJVertex> OBJvertices; //.obj頂点データ
	std::vector<unsigned> indices; //インデックス

	ID3D12Resource* vertexBuffer = nullptr; //頂点バッファ
	ID3D12Resource* indexBuffer = nullptr; //インデックスバッファ
	ID3D12Resource* materialBuffer = nullptr; //マテリアルバッファ
	D3D12_VERTEX_BUFFER_VIEW vbView = {}; //自身の持つバッファのビュー
	D3D12_INDEX_BUFFER_VIEW ibView = {}; //自身の持つバッファのビュー

	/*-----コンストラクタ/デストラクタ-----*/
	Object(); 
	Object(std::string name);
	~Object();

	/*-----メンバ関数-----*/
	HRESULT LoadOBJData(std::string ModelName, ID3D12Device* device); //OBJモデル読み込み
	std::vector<std::string> split(std::string str, std::string splitter); //文字列分割(分割する文字列, 分割ワード)
	template<typename T>
	void vectorRelease(std::vector<T>& vec); //vector解放関数
	void Release(); //インスタンス解放関数
};