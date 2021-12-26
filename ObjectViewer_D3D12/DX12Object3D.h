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
	std::string m_name; //�I�u�W�F�N�g��

	/*-----���_�o�b�t�@-----*/
	ID3D12Resource* m_vertexBuffer = nullptr;
	std::vector<DX12Vertex> m_vertices;
	D3D12_VERTEX_BUFFER_VIEW m_vbv = {};

	/*-----�C���f�b�N�X�o�b�t�@-----*/
	ID3D12Resource* m_indexBuffer = nullptr;
	std::vector<unsigned> m_indices;
	D3D12_INDEX_BUFFER_VIEW m_ibv = {};
	
	/*-----�}�e���A���o�b�t�@-----*/
	ID3D12Resource* m_materialBuffer = nullptr;
	DX12Material m_material;
	ID3D12DescriptorHeap* m_matDescHeap = nullptr;

	/*-----�e�N�X�`���o�b�t�@-----*/
	ID3D12Resource* m_textureBuffer = nullptr;
	ID3D12DescriptorHeap* m_texDescHeap = nullptr;

	/*-----�ʒu�p���傫��-----*/
	DX12Transform m_transform;


private :
	HRESULT CreateVertexBuffer(ID3D12Device* device); //���_�o�b�t�@�̐���
	HRESULT CreateIndexBuffer(ID3D12Device* device); //�C���f�b�N�X�o�b�t�@�̐���
	HRESULT CreateMaterialBuffer(ID3D12Device* device); //�}�e���A���o�b�t�@�̐���
	HRESULT CreateTextureBuffer(ID3D12Device* device, std::wstring texFilePath); //�e�N�X�`���o�b�t�@�̐���
	HRESULT LoadTexture(std::wstring texFilePath, ScratchImage& img, TexMetadata& md);

public :
	HRESULT Create(ID3D12Device* device); //�o�b�t�@�𐶐�����
	HRESULT Draw(ID3D12GraphicsCommandList* cmdList, ID3D12DescriptorHeap* cbvHeap, UINT cbOffset); //�R�}���h���X�g�Ƀf�[�^���Z�b�g���ĕ`�悷��
	void UpdateMaterial(float* data);
	void Release(); //���I�Ɋm�ۂ������������J������
};