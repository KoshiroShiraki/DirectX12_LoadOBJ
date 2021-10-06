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
	mcb.ambient = XMFLOAT3(0.0f, 0.0f, 0.0f);
	mcb.diffuse = XMFLOAT3(0.0f, 0.0f, 0.0f);
	mcb.specular = XMFLOAT3(0.0f, 0.0f, 0.0f);
	mcb.Nspecular = 0.0f;

	ambTexPath = "noTex";
	difTexPath = "noTex";
	speTexPath = "noTex";
}

OBJMaterialRef::OBJMaterialRef() {
	existFlag = 0;
}

OBJMaterialRef::~OBJMaterialRef() {
	idxOffset.clear();
	idxOffset.shrink_to_fit();
}

OBJMaterialRef::OBJMaterialRef(const OBJMaterialRef& mr) {
	this->matName = mr.matName;
	for (int mrCnt = 0; mrCnt < mr.idxOffset.size(); mrCnt++) {
		this->idxOffset.push_back(mr.idxOffset[mrCnt]);
	}
	this->existFlag = mr.existFlag;
}

Object::Object() {

}

Object::Object(std::string name) {
	this->objName = name;
}

Object::~Object() {
	Release();
}

HRESULT Object::OBJ_LoadModelData(std::string modelPath, ID3D12Device* device) {
	//オブジェクトの多重ロードを防ぐ
	if (ObjectLoaded) {
		std::cout << "Object File is already loaded\n";
		return S_OK;
	}
	ObjectLoaded = true;
	std::cout << "\n-------------------------\n";
	std::cout << "Loading Model(start)" << std::endl;

	HRESULT hr;

	float loadTime = clock(); //ロード開始時間を取得

	//ファイルロード
	hr = checkPathLength(modelPath.size());
	if (FAILED(hr)) return S_FALSE;
	FILE* modelFp; //モデルファイル
	modelFp = fopen(modelPath.c_str(), "r");
	if (modelFp == NULL) {
		std::cout << "cannnot open model file\n";
		fclose(modelFp);
		return S_FALSE;
	}

	//メッシュファイルパスから相対パスを作る
	int relativePos = modelPath.find_last_of("/"); //相対パス位置
	std::string matPath = modelPath.substr(0, relativePos);
 	
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
	OBJファイルは面データの参照を1スタートで行う
	要素[0]にデフォルト値を代入して起き、面データの対応がないときは、頂点にこの値を代入する
	*/
	v.push_back(XMFLOAT3(0.0f,0.0f,0.0f));
	vt.push_back(XMFLOAT2(0.0f, 0.0f));
	vn.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));

	char lineData[MAX_READ_LINEDATA]; //1行データ
	
	std::vector<std::string> splitData; //分割後文字列格納配列
	splitData.reserve(50); //メモリ再確保防止

	bool isMaterial = false; //マテリアルファイルが存在するかフラグ

	std::map<std::string, std::vector<int>> matChecker; //usemtl 〇〇の〇〇が重複することがあるので、監視


	std::cout << "Mesh File : [" << modelPath << "]" << std::endl;

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
				OBJ_splitSlash(splitData[i + 1], tmpData.dataIndex[i]);
			}
			fd.push_back(tmpData);
		}
		else if (splitData[0] == "mtllib") { //マテリアルファイルパス
			isMaterial = true; //マテリアル存在フラグを立てる
			//メッシュファイルによっては.mtlファイル名のみの記述の場合もあれば./から記述しているものもあるので対応
			int slashPos = splitData[1].find_first_of("/");
			if (slashPos != std::string::npos) splitData[1].erase(0, slashPos - 1);
			else splitData[1].insert(0, "/");

			matPath = matPath + splitData[1];
		}
		else if (splitData[0] == "usemtl") { //マテリアル名
			OBJMaterialRef tmpMatRef;
			for (int splitNum = 1; splitNum < splitData.size(); splitNum++) { //マテリアル名が空白込みで定義されている場合があるので対応
				tmpMatRef.matName += splitData[splitNum];
			}
			int idx = findMaterialIndex(matRef, tmpMatRef.matName);
			if (idx == -1) {	
				tmpMatRef.idxOffset.push_back(idxOffset);
				matRef.push_back(tmpMatRef);
			}
			else {
				matRef[idx].idxOffset.push_back(idxOffset);
			}
		}
		splitData.clear();
	}
	fclose(modelFp);

	if (isMaterial) { //この後の処理はマテリアルファイルがあった場合にのみ行う
		//ファイルロード
		hr = checkPathLength(matPath.size());
		if (FAILED(hr)) return S_FALSE;
		FILE* matFp; //マテリアルファイル
		matFp = fopen(matPath.c_str(), "r");
		if (matFp == NULL) {
			fclose(matFp);
			std::cout << "cannnot open material file\n";
		}

		std::cout << "Material File : [" << matPath << "]" << std::endl;
		
		/*
		マテリアルファイルは、newmtlから次の空行まで
		*/
		for (int matNum = 0; matNum < matRef.size(); matNum++) {
			while (fgets(lineData, MAX_READ_LINEDATA, matFp) != NULL) { //メッシュファイルで宣言されていたマテリアルがマテリアルファイルにない可能性がある。その場合はこのループを通過することになる
				OBJ_splitBlank(lineData, splitData);
				if (splitData[0] == "newmtl") { //マテリアルファイルでのマテリアル宣言
					std::string tmpName;
					for (int splitNum = 1; splitNum < splitData.size(); splitNum++) {
						tmpName += splitData[splitNum];
					}

					if (matRef[matNum].matName == tmpName) { //マテリアル参照データに格納されているマテリアル名とnewmtlで宣言されているマテリアル名が一致した場合
						matRef[matNum].existFlag = 1; //マテリアルがマテリアルファイル内で見つかった

						OBJMaterial tmpMat;
						fgets(lineData, MAX_READ_LINEDATA, matFp);
						OBJ_splitBlank(lineData, splitData);
						while (splitData[0] != "newmtl") { //つぎのnewmtl宣言まで
							if (splitData[0] == "Ka") { //アンビエント色
								tmpMat.mcb.ambient = XMFLOAT3(std::stof(splitData[1]), std::stof(splitData[2]), std::stof(splitData[3]));
							}
							else if (splitData[0] == "Kd") { //ディフューズ色
								tmpMat.mcb.diffuse = XMFLOAT3(std::stof(splitData[1]), std::stof(splitData[2]), std::stof(splitData[3]));
							}
							else if (splitData[0] == "Ks") { //スペキュラ色
								tmpMat.mcb.specular = XMFLOAT3(std::stof(splitData[1]), std::stof(splitData[2]), std::stof(splitData[3]));
							}
							else if (splitData[0] == "Ns") { //スペキュラ指数
								tmpMat.mcb.Nspecular = std::stof(splitData[1]);
							}
							else if (splitData[0] == "map_Ka") { //アンビエントテクスチャ
								tmpMat.ambTexPath = splitData[1];
							}
							else if (splitData[0] == "map_Kd") { //ディフューズテクスチャ
								tmpMat.difTexPath = splitData[1];
							}
							else if (splitData[0] == "map_Ks") { //スペキュラテクスチャ
								tmpMat.speTexPath = splitData[1];
							}

							//行を進めて分割
							if (fgets(lineData, MAX_READ_LINEDATA, matFp) == NULL) break;
							OBJ_splitBlank(lineData, splitData);
						}
						materials.push_back(tmpMat); //マテリアルデータの追加
						rewind(matFp);
						break;
					}
				}
			}
		}
		for (int i = 0; i < matRef.size(); i++) {
			if (matRef[i].existFlag == 0) {
				std::cout << "warning : no material : " << matRef[i].matName << std::endl;
			}
		}

		fclose(matFp);

		/*for (int i = 0; i < materials.size(); i++) {
			std::cout << matRef[i].matName << std::endl;
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

	std::cout << "vertex : " << vertices.size() << " / UV : " << vt.size() << " / normal : " << vn.size() << " / face : " << fd.size() << " / indices : " << indices.size() << " / idxOffset : " << idxOffset << "\n";

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
	for (int i = 0; i < 3; i++) {
		std::cout << &matMap[i].ambient.x << "/" << &matMap[i].ambient.y << "/" << &matMap[i].ambient.z << std::endl;
		std::cout << &matMap[i].diffuse.x << "/" << &matMap[i].diffuse.y << "/" << &matMap[i].diffuse.z << std::endl;
		std::cout << &matMap[i].specular.x << "/" << &matMap[i].specular.y << "/" << &matMap[i].specular.z << std::endl;
		std::cout << &matMap[i].Nspecular << std::endl << std::endl;
	}
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
		return S_FALSE;
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
	vectorRelease(vertices);
	vectorRelease(indices);
	vectorRelease(materials);
	vectorRelease(matRef);

	if (vertexBuffer) vertexBuffer->Release();
	if (indexBuffer) indexBuffer->Release();
	if (materialBuffer) materialBuffer->Release();
}