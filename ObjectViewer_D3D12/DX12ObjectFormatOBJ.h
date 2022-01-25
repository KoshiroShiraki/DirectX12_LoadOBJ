#pragma once
#include<time.h>
#include"DX12Object3D.h"
#include"PathController.h"
#include<fstream>

#define MAX_COUNT_LINEDATA 100000

class DX12ObjectFormatOBJ {
public :
	DX12ObjectFormatOBJ();
	DX12ObjectFormatOBJ(DX12ObjectFormatOBJ* origin, ID3D12Device* device); //既存のモデルのパラメータを使用する場合
	~DX12ObjectFormatOBJ();

public :
	std::vector<DX12Object3D*> m_obj;
	std::string m_name;

	DX12Transform m_transform;
	XMMATRIX m_wMatrix;


	HRESULT LoadVertexFromOBJFile(std::string filePath, ID3D12Device* device); //OBJファイルからの読み込み
	HRESULT SaveFMDFile(); //Flatbuffersによってシリアライズされたデータをfmdファイルとして保存する
	void splitBlank(std::string str, std::vector<std::string>& data);
	void splitSlash(std::string str, int* data);
	void UpdateTransform(float* data); //位置姿勢大きさのアップデート ID
};