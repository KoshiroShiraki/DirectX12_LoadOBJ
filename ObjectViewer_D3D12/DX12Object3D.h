#pragma once
#include<d3d12.h>
#include<d3dx12.h>
#include<DirectXTex.h>
#include<DirectXMath.h>
#include<Windows.h>
#include<iostream>
#include"ErrorDebug.h"

using namespace DirectX;

struct DX12Vertex {
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT2 uv;
};

struct DX12Material {
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;
	float N;

	char padding[204];
};

struct DX12Transform {
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	XMFLOAT3 size;
};

class DX12Object3D {
public :
	DX12Object3D();
	~DX12Object3D();

public :
	std::string m_name; //オブジェクト名

	/*-----頂点バッファ-----*/
	ID3D12Resource* m_vertexBuffer = nullptr;
	std::vector<DX12Vertex> m_vertices;
	D3D12_VERTEX_BUFFER_VIEW m_vbv = {};

	/*-----インデックスバッファ-----*/
	ID3D12Resource* m_indexBuffer = nullptr;
	std::vector<unsigned> m_indices;
	D3D12_INDEX_BUFFER_VIEW m_ibv = {};
	
	/*-----マテリアルバッファ-----*/
	ID3D12Resource* m_materialBuffer = nullptr;
	DX12Material m_material;
	ID3D12DescriptorHeap* m_matDescHeap = nullptr;

	/*-----テクスチャバッファ-----*/
	ID3D12Resource* m_textureBuffer = nullptr;
	ID3D12DescriptorHeap* m_texDescHeap = nullptr;

	/*-----位置姿勢大きさ-----*/
	DX12Transform m_transform;


private :
	HRESULT CreateVertexBuffer(ID3D12Device* device); //頂点バッファの生成
	HRESULT CreateIndexBuffer(ID3D12Device* device); //インデックスバッファの生成
	HRESULT CreateMaterialBuffer(ID3D12Device* device); //マテリアルバッファの生成
	HRESULT CreateTextureBuffer(ID3D12Device* device, std::wstring texFilePath); //テクスチャバッファの生成
	HRESULT LoadTexture(std::wstring texFilePath, ScratchImage& img, TexMetadata& md);

public :
	HRESULT Create(ID3D12Device* device); //バッファを生成する
	HRESULT Draw(ID3D12GraphicsCommandList* cmdList, ID3D12DescriptorHeap* cbvHeap, UINT cbOffset); //コマンドリストにデータをセットして描画する
	void UpdateMaterial(float* data);
	void Release(); //動的に確保したメモリを開放する
};