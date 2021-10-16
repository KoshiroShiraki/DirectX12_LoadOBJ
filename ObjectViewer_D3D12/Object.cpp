#include"Object.h"
#pragma warning(disable: 4996)

OBJFaceData::OBJFaceData(int cnt) {
	vertexCnt = cnt;
	//面を構成する頂点数 * 3個の二重配列を用意する
	dataIndex = (int**)malloc(sizeof(int*) * cnt);
	for (int i = 0; i < cnt; i++) {
		dataIndex[i] = (int*)malloc(sizeof(int) * 3);
	}
}

OBJFaceData::~OBJFaceData() {
	for (int i = 0; i < vertexCnt - 1; i++) {
		if (dataIndex[i] != NULL) free(dataIndex[i]);
		dataIndex[i] = NULL; //二重解放対策
	}
	if (dataIndex != NULL) free(dataIndex);
	dataIndex = NULL; //二重解放対策
}

OBJFaceData::OBJFaceData(const OBJFaceData& fd) {
	this->vertexCnt = fd.vertexCnt;

	//ちゃんとコピーはコピーで別のメモリを確保してもらわないと困る
	this->dataIndex = (int**)malloc(sizeof(int*) * this->vertexCnt);
	for (int i = 0; i < this->vertexCnt; i++) {
		this->dataIndex[i] = (int*)malloc(sizeof(int) * 3);
		for (int j = 0; j < 3; j++) {
			dataIndex[i][j] = fd.dataIndex[i][j];
		}
	}
}

OBJMaterial::OBJMaterial() {
	Init();
}
void OBJMaterial::Init() {
	mcb.ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mcb.diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mcb.specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mcb.Nspecular = 0.0f;

	ambTexPath[0] = '\0';
	difTexPath[0] = '\0';
	speTexPath[0] = '\0';
}

OBJMaterialRef::OBJMaterialRef() {
	matID = 0;
}

Object::Object() {
	ObjectLoaded = false;

	transform.position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	transform.rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	transform.size = XMFLOAT3(1.0f, 1.0f, 1.0f);

}

Object::Object(std::string name) {
	this->objName = name;
}

Object::~Object() {
	Release();
}

HRESULT Object::OBJ_LoadModelData(std::string path, ID3D12Device* device) {
	//オブジェクトの多重ロードを防ぐ
	if (ObjectLoaded) {
		std::cout << "Object File is already loaded\n";
		return E_FAIL;
	}
	ObjectLoaded = true;
	std::cout << "\n-------------------------\n";
	std::cout << "Loading Model(start)" << std::endl;

	HRESULT hr;
	
	PathController pc; //パスコントローラ

	char modelPath[MAX_PATH_LENGTH]; //モデルファイルパス
	char meshPath[MAX_PATH_LENGTH]; //メッシュファイルパス
	char materialPath[MAX_PATH_LENGTH]; //マテリアルファイルパス
	char modelDirName[64]; //モデルファイル名(メッシュファイルパス, テクスチャファイルパスの生成に使う)

	//メッシュファイルパスの生成
	pc.CreatePath(path.c_str(), meshPath);
	//モデルファイルのパスの生成(メッシュファイルパスの親ディレクトリがモデルファイルなので、メッシュファイルパスからメッシュファイル名.拡張子を削除する)
	pc.RemoveLeafPath(meshPath, modelPath);
	//モデルファイル名の取得
	pc.GetLeafDirectryName(modelPath, modelDirName, 64);

	/*std::cout << modelPath << std::endl;
	std::cout << meshPath << std::endl;
	std::cout << modelDirName << std::endl;*/

	float loadTime = clock(); //ロード開始時間を取得

	//ファイルロード
	FILE* modelFp; //モデルファイル
	modelFp = fopen(meshPath, "r");
	if (modelFp == NULL) {
		std::cout << "cannnot open model file\n";
		return E_FAIL;
	}
 	
	//一行ずつ読み込み、[v],[vt],[vn],[f]を取得
	std::vector<XMFLOAT3> v; //頂点座標
	std::vector<XMFLOAT2> vt; //テクスチャ座標
	std::vector<XMFLOAT3> vn; //法線情報
	std::vector<OBJFaceData> fd; //面データ

	//capacityをこの時点で動的確保しておき、余分なメモリの再確保を防ぐ
	v.reserve(524288);
	vt.reserve(524288);
	vn.reserve(524288);
	fd.reserve(524288);
	
	/*
	OBJファイルは面データの頂点カウントを1スタートで行う
	要素[0]にデフォルト値を代入して起き、面データの対応がないとき(v//とかv/vt/とか)は、頂点にこの値を代入する
	*/
	v.push_back(XMFLOAT3(0.0f,0.0f,0.0f));
	vt.push_back(XMFLOAT2(0.0f, 0.0f));
	vn.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));

	char lineData[MAX_READ_LINEDATA]; //1行データ
	
	std::vector<std::string> splitData; //分割後文字列格納配列
	splitData.reserve(50); //メモリ再確保防止

	bool isMaterial = false; //マテリアルファイルが存在するかフラグ

	int idxOffset = 0; //インデックスオフセット 3頂点の三角形ポリゴンメッシュで描画していくので、頂点オフセットは (面で宣言される頂点数 - 2) * 3ストライドする必要あり

	//1行ずつ文字列で取得、分割＆変換しデータとして記録していく
	while (fgets(lineData, MAX_READ_LINEDATA, modelFp) != NULL) {
		//読み込んだ一行のデータを分割し、配列に格納できる形にする
		OBJ_splitBlank(lineData, splitData);
		if (splitData[0] == "v") { //頂点
			v.push_back(XMFLOAT3(std::stof(splitData[1]), std::stof(splitData[2]), std::stof(splitData[3])));
		}
		else if (splitData[0] == "vt") { //UV
			vt.push_back(XMFLOAT2(std::stof(splitData[1]), std::stof(splitData[2])));
		}
		else if (splitData[0] == "vn") { //法線
			vn.push_back(XMFLOAT3(std::stof(splitData[1]), std::stof(splitData[2]), std::stof(splitData[3])));
		}
		else if (splitData[0] == "f") { //面データ
			idxOffset += (((splitData.size() - 1) - 2) * 3);

			OBJFaceData tmpData(splitData.size() - 1);
			for (int i = 0; i < tmpData.vertexCnt; i++) {
				OBJ_splitSlash(splitData[i + 1], tmpData.dataIndex[i]); //面データはスラッシュ区切り
			}
			fd.push_back(tmpData);
		}
		else if (splitData[0] == "mtllib") { //マテリアルファイルパス
			isMaterial = true; //マテリアル存在フラグを立てる
			//マテリアルファイル名の記述に./から始めている場合はそれを削除する
			int slashPos = splitData[1].find_first_of("/");
			if (slashPos != std::string::npos) splitData[1].erase(0, slashPos + 1);

			pc.AddLeafPath(modelPath, materialPath, splitData[1].c_str()); //モデルファイルパスにマテリアルファイルパスを付加する
			std::cout << materialPath << std::endl;
		}
		else if (splitData[0] == "usemtl") { //マテリアル名
			OBJMaterialRef tmpMatRef;
			for (int splitNum = 1; splitNum < splitData.size(); splitNum++) { //マテリアル名が空白込みで定義されている場合があるので対応
				tmpMatRef.matName += splitData[splitNum];
			}
			tmpMatRef.idxOffset = idxOffset;
			matRef.push_back(tmpMatRef);
		}
		splitData.clear();
	}
	fclose(modelFp);
	
	//マテリアル適用数を決める
	for (int i = 0; i < matRef.size() - 1; i++) {
		matRef[i].idxNum = matRef[i + 1].idxOffset - matRef[i].idxOffset;

		/*std::cout << matRef[i].matName << std::endl;
		std::cout << matRef[i].idxOffset << std::endl;
		std::cout << matRef[i].idxNum << std::endl;*/
	}
	matRef[matRef.size() - 1].idxNum = idxOffset - matRef[matRef.size() - 1].idxOffset;

	/*std::cout << matRef[matRef.size() - 1].matName << std::endl;
	std::cout << matRef[matRef.size() - 1].idxOffset << std::endl;
	std::cout << matRef[matRef.size() - 1].idxNum << std::endl;*/

	if (isMaterial) { //この後の処理はマテリアルファイルがあった場合にのみ行う
		//ファイルロード
		FILE* matFp; //マテリアルファイル
		matFp = fopen(materialPath, "r");
		if (matFp == NULL) {
			std::cout << "cannnot open material file\n";
		}

		//std::cout << materialPath << std::endl;

		/*
		マテリアルファイルは、newmtlから次の空行まで
		*/
		int materialNum = 0;
		OBJMaterial tmpMat;
		std::string tmpName;
		while (fgets(lineData, MAX_READ_LINEDATA, matFp) != NULL) {
			OBJ_splitBlank(lineData, splitData);
			if (splitData[0] == "newmtl") { //マテリアルファイルでのマテリアル宣言
				if (materialNum > 0) materials.push_back(tmpMat);
				tmpMat.Init();
				tmpName.clear();
				for (int splitNum = 1; splitNum < splitData.size(); splitNum++) {
					tmpName += splitData[splitNum];
				}
				tmpMat.materialName = tmpName;
				//マテリアル参照データと実際のマテリアルの対応付けを行う
				for (int i = 0; i < matRef.size(); i++) {
					if (matRef[i].matName == tmpName) {
						matRef[i].matID = materialNum;
					}
				}
				materialNum++;
			}
			else {
				if (splitData[0] == "Ka") { //アンビエント色
					tmpMat.mcb.ambient = XMFLOAT4(std::stof(splitData[1]), std::stof(splitData[2]), std::stof(splitData[3]), 1.0f);
				}
				else if (splitData[0] == "Kd") { //ディフューズ色
					tmpMat.mcb.diffuse = XMFLOAT4(std::stof(splitData[1]), std::stof(splitData[2]), std::stof(splitData[3]), 1.0f);
				}
				else if (splitData[0] == "Ks") { //スペキュラ色
					tmpMat.mcb.specular = XMFLOAT4(std::stof(splitData[1]), std::stof(splitData[2]), std::stof(splitData[3]), 1.0f);
				}
				else if (splitData[0] == "Ns") { //スペキュラ指数
					tmpMat.mcb.Nspecular = std::stof(splitData[1]);
				}


				//ファイルパスを適切な形に変換する必要あり
				/*
				1.まずモデルファイルディレクトリ名で検索
				2.見つかった場合は、そのファイルディレクトリ以下のパスを取得し、modelPathに結合する
				3.見つからない場合は、フォルダ分けされずに直接モデルファイル直下に置かれているとみなして、文字列をそのままmodelPathに結合する
				*/
				else if (splitData[0] == "map_Ka") { //アンビエントテクスチャ
					std::string tmpMatPath;
					for (int i = 1; i < splitData.size(); i++) { //今回の結合は空白を空白として扱う
						tmpMatPath += splitData[i];
						tmpMatPath += " ";
					}
					tmpMatPath.erase(tmpMatPath.end() - 1); //最後の空白は余計なので削除

					char tmpStr[MAX_PATH_LENGTH];
					pc.GetAfterPathFromDirectoryName(tmpMatPath.c_str(), tmpStr, modelDirName);
					pc.AddLeafPath(modelPath, tmpMat.ambTexPath, tmpStr);
				}
				else if (splitData[0] == "map_Kd") { //ディフューズテクスチャ
					std::string tmpMatPath;
					for (int i = 1; i < splitData.size(); i++) { //今回の結合は空白を空白として扱う
						tmpMatPath += splitData[i];
						tmpMatPath += " ";
					}
					tmpMatPath.erase(tmpMatPath.end() - 1); //最後の空白は余計なので削除

					char tmpStr[MAX_PATH_LENGTH];
					pc.GetAfterPathFromDirectoryName(tmpMatPath.c_str(), tmpStr, modelDirName);
					pc.AddLeafPath(modelPath, tmpMat.difTexPath, tmpStr);
				}
				else if (splitData[0] == "map_Ks") { //スペキュラテクスチャ
					std::string tmpMatPath;
					for (int i = 1; i < splitData.size(); i++) { //今回の結合は空白を空白として扱う
						tmpMatPath += splitData[i];
						tmpMatPath += " ";
					}
					tmpMatPath.erase(tmpMatPath.end() - 1); //最後の空白は余計なので削除
					char tmpStr[MAX_PATH_LENGTH];

					pc.GetAfterPathFromDirectoryName(tmpMatPath.c_str(), tmpStr, modelDirName);
					pc.AddLeafPath(modelPath, tmpMat.speTexPath, tmpStr);
				}
			}
		}
		materials.push_back(tmpMat);
	
		/*for (int i = 0; i < matRef.size(); i++) {
			std::cout << matRef[i].matName << " " << matRef[i].matID << std::endl;
		}*/

		fclose(matFp);

		/*for (int i = 0; i < materials.size(); i++) {
			std::cout << materials[i].materialName << std::endl;
			std::cout << "ambient : " << materials[i].mcb.ambient.x << " ";
			std::cout << materials[i].mcb.ambient.y << " ";
			std::cout << materials[i].mcb.ambient.z << std::endl;
			std::cout << "diffuse : " << materials[i].mcb.diffuse.x << " ";
			std::cout << materials[i].mcb.diffuse.y << " ";
			std::cout << materials[i].mcb.diffuse.z << " " << std::endl;
			std::cout << "specular : " << materials[i].mcb.specular.x << " ";
			std::cout << materials[i].mcb.specular.y << " ";
			std::cout << materials[i].mcb.specular.z << " " << std::endl;
			std::cout << "Nspecular : " << materials[i].mcb.Nspecular << std::endl;
			std::cout << "ambient texture : " << materials[i].ambTexPath << std::endl;
			std::cout << "diffuse texture : " << materials[i].difTexPath << std::endl;
			std::cout << "specular texture : " << materials[i].speTexPath << std::endl;
			std::cout << std::endl;
		}*/
	}

	unsigned indexNum = 0; //インデックス数 1頂点処理するたびにインクリメントする
	//面情報をもとに、　頂点情報の格納と頂点インデックスの生成;
	for (int i = 0; i < fd.size(); i++) {
		//この時点でマテリアル参照データのインデックスオフセットは面の番号で記録されているので、ここでインデックス番号に修正する
		

		/*
		"/"ない場合 : 頂点のみ
		"a/b"の場合 : 頂点/テクスチャ座標
		"a//b"の場合 : 頂点//法線
		"a/b/c"の場合 : 頂点/テクスチャ座標/法線
		*/

		//まず面情報を分割する
		/*
		objファイルは、面情報(f)に頂点と頂点に対応するUV座標および法線の情報がある
	    fの一要素が一頂点となる
		*/

		int splitData[3];
		for (int j = 0; j < fd[i].vertexCnt; j++) {
			OBJVertex tmpVert; //OBJvertices一時保管用

			tmpVert.pos = v[fd[i].dataIndex[j][0]];
			tmpVert.uv = vt[fd[i].dataIndex[j][1]];
			tmpVert.normal = vn[fd[i].dataIndex[j][2]];
				
			//頂点データへプッシュ
			vertices.push_back(tmpVert);
		}

		//f[i].faceNumの個数をもとにインデックス値を生成
		int indexOffset = indexNum;
		for (int j = 0; j < (fd[i].vertexCnt - 2); j++) {
			indices.push_back(indexOffset);
			indices.push_back(indexOffset + 1 + j);
			indices.push_back(indexOffset + 2 + j);
		}

		indexNum += fd[i].vertexCnt;
	}

	std::cout << "vertex : " << vertices.size() << " / UV : " << vt.size() << " / normal : " << vn.size() << " / face : " << fd.size() << " / indices : " << indices.size() << std::endl;

	vectorRelease(v);
	vectorRelease(vt);
	vectorRelease(vn);
	vectorRelease(fd);
	vectorRelease(splitData);

	//HeapPropertyの設定
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	//ResourcDescの定義
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = vertices.size() * sizeof(OBJVertex);
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count = 1;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	/*-----VertexBufferの生成-----*/
	//VertexBuffer(Resource)の生成
	hr = device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexBuffer));
	if (FAILED(hr)) {
		std::cout << "Failed to Create vertexBuffer\n";
		return hr;
	}
	/*-----頂点データのマップ-----*/
	OBJVertex* vertMap = nullptr;
	hr = vertexBuffer->Map(0, nullptr, (void**)&vertMap);
	if (FAILED(hr)) {
		std::cout << "Failed to Map vertex\n";
		return hr;
	}
	std::copy(std::begin(vertices), std::end(vertices), vertMap);
	vertexBuffer->Unmap(0, nullptr);

	/*-----VertexBufferViewの生成-----*/
	vbView = {};
	vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vbView.SizeInBytes = vertices.size() * sizeof(OBJVertex);
	vbView.StrideInBytes = sizeof(OBJVertex);

	/*-----IndexBufferの生成-----*/
	//ResourceDescの変更
	resDesc.Width = indices.size() * sizeof(indices[0]);
	//IndexBufferの生成
	hr = device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&indexBuffer));
	if (FAILED(hr)) {
		std::cout << "Failed to Create indexBuffer\n";
		return hr;
	}
	/*-----インデックスデータのマップ-----*/
	unsigned* idxMap = nullptr;
	hr = indexBuffer->Map(0, nullptr, (void**)&idxMap);
	if (FAILED(hr)) {
		std::cout << "Failed to Map index\n";
		return hr;
	}
	std::copy(std::begin(indices), std::end(indices), idxMap);
	indexBuffer->Unmap(0, nullptr);

	/*-----IndexBufferViewの生成-----*/
	ibView = {};
	ibView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R32_UINT;
	ibView.SizeInBytes = indices.size() * sizeof(indices[0]);

	/*-----MaterialBufferの生成-----*/
	//ResourceDescの変更
	resDesc.Width = sizeof(OBJMaterialCB) * materials.size();
	//MaterialBufferの生成
	hr = device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&materialBuffer));
	if (FAILED(hr)) {
		std::cout << "Failed to Create materialBuffer" << std::endl;
	}
	std::cout << materialBuffer->GetDesc().Width << std::endl;
	/*-----マテリアルデータのマップ-----*/
	OBJMaterialCB* matMap;
	hr = materialBuffer->Map(0, nullptr, (void**)&matMap);
	if (FAILED(hr)) {
		std::cout << "Failed to Map material" << std::endl;
		return hr;
	}
	for (int i = 0; i < materials.size(); i++) {
		memcpy((matMap + i), &materials[i].mcb, sizeof(OBJMaterialCB));
	}
	/*for (int i = 0; i < materials.size(); i++) {
		std::cout << &matMap[i].ambient.x << "/" << &matMap[i].ambient.y << "/" << &matMap[i].ambient.z << std::endl;
		std::cout << &matMap[i].diffuse.x << "/" << &matMap[i].diffuse.y << "/" << &matMap[i].diffuse.z << std::endl;
		std::cout << &matMap[i].specular.x << "/" << &matMap[i].specular.y << "/" << &matMap[i].specular.z << std::endl;
		std::cout << &matMap[i].Nspecular << std::endl << std::endl;
	}*/
	/*for (int i = 0; i < materials.size(); i++) {
		std::cout << matMap[i].ambient.x << "/" << matMap[i].ambient.y << "/" << matMap[i].ambient.z << std::endl;
		std::cout << matMap[i].diffuse.x << "/" << matMap[i].diffuse.y << "/" << matMap[i].diffuse.z << std::endl;
		std::cout << matMap[i].specular.x << "/" << matMap[i].specular.y << "/" << matMap[i].specular.z << std::endl;
		std::cout << matMap[i].Nspecular << std::endl << std::endl;
	}*/
	materialBuffer->Unmap(0, nullptr);

	/*-----マテリアル用ディスクリプタヒープの生成-----*/
	D3D12_DESCRIPTOR_HEAP_DESC matHeapDesc;
	matHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	matHeapDesc.NodeMask = 0;
	matHeapDesc.NumDescriptors = materials.size();
	matHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	hr = device->CreateDescriptorHeap(&matHeapDesc, IID_PPV_ARGS(&materialDescHeap));
	if (FAILED(hr)) {
		std::cout << "Failed to Create material Descriptor Heap" << std::endl;
		return hr;
	}
	//マテリアルバッファビューを作る(バッファのアドレス(位置)を教えなきゃいけないので、マテリアル数分のビューが必要らしい)
	D3D12_CONSTANT_BUFFER_VIEW_DESC matCBVDesc = { };
	matCBVDesc.BufferLocation = materialBuffer->GetGPUVirtualAddress();
	matCBVDesc.SizeInBytes = sizeof(OBJMaterialCB);
	D3D12_CPU_DESCRIPTOR_HANDLE handleOffset = materialDescHeap->GetCPUDescriptorHandleForHeapStart();
	for (int i = 0; i < materials.size(); i++) {
		device->CreateConstantBufferView(&matCBVDesc, handleOffset);
		handleOffset.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		matCBVDesc.BufferLocation += sizeof(OBJMaterialCB);
	}


	std::cout << "Loading Model is finished(" << (clock() - loadTime) / 1000 << "sec)" << std::endl;
	
	std::cout << "-------------------------\n";

	return S_OK;
}

HRESULT Object::checkPathLength(size_t length) {
	if (length > MAX_PATH_LENGTH) {
		std::cout << "Error : Path is too long" << std::endl;
		return E_FAIL;
	}
	return S_OK;
}

void Object::OBJ_splitBlank(std::string str, std::vector<std::string>& data) {
	data.clear();

	std::string splitter = " "; //分割文字列
	std::string tmp; //一時格納用文字列

	int offset = 0;
	int splitPos = 0;

	splitPos = str.find(splitter, offset);
	while (splitPos != -1) {
		tmp = str.substr(offset, splitPos - offset);

		if (tmp.size() != 0) data.push_back(tmp); //空白と空白の間に文字列がない場合にはプッシュしない

		offset = splitPos + 1; //探索開始点を進める
		splitPos = str.find(splitter, offset);
	}

	if (str.substr(offset).size() != 0) data.push_back(str.substr(offset, (str.size() - offset - 1))); //最後の一回(エスケープシーケンス\nが入るので無視するために1要素省く)
}

void Object::OBJ_splitSlash(std::string str, int* data) {
	std::string splitter = "/"; //分割文字列
	std::string tmp; //一時格納用文字列

	int offset = 0;
	int splitPos = 0;

	splitPos = str.find(splitter, offset);
	for(int i = 0;i < 2;i++){
		tmp = str.substr(offset, splitPos - offset);

		if (tmp.size() == 0) data[i] = 0; //スラッシュとスラッシュの間に文字がない場合は、0をプッシュする
		else data[i] = std::stoi(tmp);

		offset = splitPos + 1; //探索開始点を進める
		splitPos = str.find(splitter, offset);
	}

	if (str.substr(offset).size() == 0) data[2] = 0; //最後の一回
	else data[2] = std::stoi((str.substr(offset)));
}

int Object::findMaterialIndex(std::vector<OBJMaterialRef> mr, std::string material) {
	for (int i = 0; i < mr.size(); i++) {
		if (mr[i].matName == material) return i;
	}

	return -1; //まだ未参照のマテリアルである場合は-1を返す
}



template<typename T>
void Object::vectorRelease(std::vector<T>& vec) {
	//最近のvectorの解放(sizeを0にしてからcapacityを0に)の仕方らしい
	vec.clear();
	vec.shrink_to_fit();
}

void Object::Release() {
	ObjectLoaded = false;

	vectorRelease(vertices);
	vectorRelease(indices);
	vectorRelease(materials);
	vectorRelease(matRef);

	if (vertexBuffer) vertexBuffer->Release();
	if (indexBuffer) indexBuffer->Release();
	if (materialBuffer) materialBuffer->Release();
}