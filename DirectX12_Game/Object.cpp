#include"Object.h"
#pragma warning(disable: 4996)

Object::Object() {

}

Object::~Object() {
	Release();
}

HRESULT Object::LoadPMDData(std::string ModelName, ID3D12Device* device) {
	if (ObjectLoaded) {
		std::cout << "Object File is already loaded\n";
		return S_OK;
	}
	ObjectLoaded = true; //オブジェクトファイルを多重ロードしない
	HRESULT hr;

	/*-----ヘッダ情報の取得-----*/
	PMDHeader pmdheader;
	char signature[3] = {};
	std::string strModelPath = "Model/" + ModelName + ".pmd";
	auto fp = fopen(strModelPath.c_str(), "rb");
	fread(signature, sizeof(signature), 1, fp);
	fread(&pmdheader, sizeof(pmdheader), 1, fp);
	std::cout << "読み込みモデル情報\n" << "バージョン : " << pmdheader.version << "\n" << "モデル名 : " << pmdheader.model_name << "\n" << "コメント : " << pmdheader.comment << "\n";

	/*-----頂点数の確認-----*/
	unsigned int vertNum;
	fread(&vertNum, sizeof(vertNum), 1, fp);
	std::cout << "モデル頂点数 : " << vertNum << "\n";

	/*-----頂点データの読み込み-----*/
	PMDvertices.resize(vertNum * pmdvertex_size);
	fread(PMDvertices.data(), PMDvertices.size(), 1, fp);

	/*-----VertexBufferの生成-----*/
	//HeapPropertyの設定
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	//ResourcDescの定義
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = PMDvertices.size();
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count = 1;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//VertexBuffer(Resource)の生成
	hr = device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexBuffer));
	if (FAILED(hr)) {
		std::cout << "Failed to Create vertexBuffer\n";
		return hr;
	}
	/*-----頂点データのマップ-----*/
	char* vertMap = nullptr;
	hr = vertexBuffer->Map(0, nullptr, (void**)&vertMap);
	if (FAILED(hr)) {
		std::cout << "Failed to Map vertex\n";
		return hr;
	}
	std::copy(std::begin(PMDvertices), std::end(PMDvertices), vertMap);
	vertexBuffer->Unmap(0, nullptr);

	/*-----VertexBufferViewの生成-----*/
	vbView = {};
	vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vbView.SizeInBytes = PMDvertices.size();
	vbView.StrideInBytes = pmdvertex_size;

	/*-----インデックス数の確認-----*/
	unsigned int indicesNum;
	fread(&indicesNum, sizeof(indicesNum), 1, fp);
	std::cout << "モデルインデックス数 : " << indicesNum << "\n";

	/*-----インデックスデータの読み込み-----*/
	indices.resize(indicesNum);
	fread(indices.data(), indices.size() * sizeof(indices[0]), 1, fp);

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
	unsigned short* idxMap = nullptr;
	hr = indexBuffer->Map(0, nullptr, (void**)&idxMap);
	if (FAILED(hr)) {
		std::cout << "Failed to Map index\n";
		return hr;
	}
	std::copy(std::begin(indices), std::end(indices), idxMap);
	indexBuffer->Unmap(0, nullptr);

	/*-----IndexBufferViewの生成　-----*/
	ibView = {};
	ibView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = indices.size() * sizeof(indices[0]);

	/*-----マテリアルデータの読み込み-----*/
	//マテリアル数の確認
	unsigned int materialNum;
	fread(&materialNum, sizeof(materialNum), 1, fp);
	std::cout << "マテリアル数 : " << materialNum << "\n";
	//マテリアルデータの読み込み
	std::vector<PMDMaterial> pmdMaterials(materialNum);
	fread(pmdMaterials.data(), pmdMaterials.size() * sizeof(PMDMaterial), 1, fp);
	//シェーダ転送用データと、CPUで参照するのみのデータに分ける
	std::vector<Material> materials(pmdMaterials.size());
	for (int i = 0; i < materials.size(); i++) {
		materials[i].indicesNum = pmdMaterials[i].indicesNum;
		materials[i].material.diffuse = pmdMaterials[i].diffuse; 
		materials[i].material.alpha = pmdMaterials[i].alpha;
		materials[i].material.specular = pmdMaterials[i].specular; 
		materials[i].material.specularity = pmdMaterials[i].specularity; 
		materials[i].material.ambient = pmdMaterials[i].ambient;
	}

	/*-----MaterialBufferの生成-----*/


	/*-----ボーン情報の読み込み-----*/
	//ボーン数の確認
	unsigned short boneNum = 0;
	fread(&boneNum, sizeof(boneNum), 1, fp);
	std::cout << "ボーン数 : " << boneNum << "\n";

	/*-----ボーンデータの読み込み-----*/
	std::vector<PMDBone> pmdBones(boneNum);
	fread(pmdBones.data(), sizeof(PMDBone), boneNum, fp);
	/*for (int i = 0; i < boneNum; i++) {
		std::cout << "ボーン[" << i << "] ボーン名 : " << pmdBones[i].boneName << "\n";
	}*/

	inputLayout = new D3D12_INPUT_ELEMENT_DESC[6];
	inputLayout[0] = { "POSITION", 0,DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputLayout[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputLayout[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputLayout[3] = { "BONE_NO", 0, DXGI_FORMAT_R16G16_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputLayout[4] = { "WEIGHT", 0, DXGI_FORMAT_R8_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputLayout[5] = { "EDGE_FLG", 0, DXGI_FORMAT_R8_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	return S_OK;
}

//各行ごとに処理
/*
objファイル
# : コメント
v : 頂点座標
vt : テクスチャ座標
vn : 法線
f : 面情報
☆読み込み方針
・1行ずつ読み込み
・最初の文字に応じて処理分岐
*/
HRESULT Object::LoadOBJData(std::string ModelName, ID3D12Device* device) {
	//オブジェクトの多重ロードを防ぐ
	if (ObjectLoaded) {
		std::cout << "Object File is already loaded\n";
		return S_OK;
	}
	ObjectLoaded = true;

	std::cout << "\n----------wavefrontOBJ読み込み開始----------\n";

	HRESULT hr;
	std::string strModelPath = "Model/" + ModelName + ".obj"; //オブジェクトファイルパスの生成
	std::ifstream file(strModelPath); //オブジェクトファイルの読み込み
	if (!file) {
		std::cout << "cannnot find model file\n";
		return FALSE;
	}

	//一行ずつ読み込み、[v],[vt],[vn],[f]を取得
	std::vector<XMFLOAT3> v; //頂点座標
	std::vector<XMFLOAT2> vt; //テクスチャ座標
	std::vector<XMFLOAT3> vn; //法線情報
	std::vector<OBJFaceInfo> f; //面情報(いったんstringのまま取得)

	FILE* fp;
	fp = fopen(strModelPath.c_str(), "r");
	if (fp == NULL) {
		std::cout << "cannnot open model file\n";
	}

	char lineData[128]; //1行データ
	//1行ずつ文字列で取得、分割＆変換しデータとして記録していく
	float time = clock();
	std::cout << "テキストデータ読み取り開始 : " << time / 1000.0f << "秒\n";
	int linecount = 0;

	float averageSplitTime = 0.0f;
	while (fgets(lineData, 128, fp) != NULL) {
		linecount++;
		//float deltaTime = clock();
		std::vector<std::string> data = split(lineData, " ");
		//averageSplitTime += clock() - deltaTime;
		if (data[0] == "v") {
			v.push_back(XMFLOAT3(std::stof(data[1]), std::stof(data[2]), std::stof(data[3])));
		}
		else if (data[0] == "vt") {
			vt.push_back(XMFLOAT2(std::stof(data[1]), std::stof(data[2])));
		}
		else if (data[0] == "vn") {
			vn.push_back(XMFLOAT3(std::stof(data[1]), std::stof(data[2]), std::stof(data[3])));
		}
		else if (data[0] == "f") {
			OBJFaceInfo info;
			for (int i = 0; i < 3; i++) {
				info.fi[i] = data[i + 1];
				//vertNum++;
			}
			//面情報の数が3の場合と4の場合
			if (data.size() == 4) {
				info.fi[3] = "Nothing";
				f.push_back(info);
			}
			else if (data.size() == 5) {
				info.fi[3] = data[4];
				//vertNum++;
				f.push_back(info);
			}
			//面情報の数が6以上の時
			else if (data.size() > 5) {
				//無視
			}
		}
	}
	std::cout << "テキストデータ読み取り終了 : " << clock() / 1000.0f << "秒\nテキストデータ読み取り時間 : " << (clock() - time) / 1000 << "秒\n";

	std::cout << "頂点数 : " << v.size() << "\n";
	int fSize = sizeof(OBJFaceInfo) / sizeof(std::string); //faceInfoのもつfiの要素数
	
	time = clock();
	std::cout << "データ解析開始 : " << time / 1000.0f << "秒\n";

	unsigned indexNum = 0; //インデックスid 一頂点処理するたびにインクリメントする
	//面情報をもとに、　頂点情報の格納と頂点インデックスの生成;
	for (int i = 0; i < f.size(); i++) {
		/*
		"/"ない場合 : 頂点のみ
		"a/b"の場合 : 頂点/テクスチャ座標
		"a//b"の場合 : 頂点//法線
		"a/b/c"の場合 : 頂点/テクスチャ座標/法線
		*/

		//まず面情報を分割する
		/*
		objファイルは、面情報(f)に頂点と頂点に対応するUV座標および法線の情報がある
		つまりfの一要素が一頂点となる
		*/

		for (int j = 0; j < fSize - 1; j++) { //f[i].fi[2]まで。[3]は"Nothing"かそれ以外かで処理分岐
			//std::cout << f[i].fi[j] << " ";
			std::vector<std::string > faceData = split(f[i].fi[j], "/");
			OBJVertex tmpVert; //OBJvertices一時保管用
			//fi[3] が nothingかそれ以外かで処理が変わる
			//nothingではない場合、4頂点であるので、三角形を生成するためにインデックスを拡張する必要がある
			
			switch (faceData.size()) {
			case 1: //頂点データのみ
				tmpVert.pos = v[std::stoi(faceData[0]) - 1]; //objファイルの頂点idはなぜか1スタートなので-1を忘れずに
				tmpVert.uv = XMFLOAT2(0.0f, 0.0f); //デフォルトは0
				tmpVert.normal = XMFLOAT3(0.0f, 0.0f, 0.0f); //デフォルトは0
				break;
			case 2: //頂点/テクスチャ座標
				tmpVert.pos = v[std::stoi(faceData[0]) - 1];
				tmpVert.uv = vt[std::stoi(faceData[1]) - 1];
				tmpVert.normal = XMFLOAT3(0.0f, 0.0f, 0.0f); //デフォルトは0
				break;
			case 3: //頂点//法線 or 頂点/テクスチャ座標/法線
				if (faceData[1].length() == 0) { //頂点//法線
					tmpVert.pos = v[std::stoi(faceData[0]) - 1];
					tmpVert.uv = XMFLOAT2(0.0f, 0.0f);
					tmpVert.normal = vn[std::stoi(faceData[2]) - 1];
				}
				else { //頂点/テクスチャ座標/法線
					tmpVert.pos = v[std::stoi(faceData[0]) - 1];
					tmpVert.uv = vt[std::stoi(faceData[1]) - 1];
					tmpVert.normal = vn[std::stoi(faceData[2]) - 1];
				}
				break;
			}
			OBJvertices.push_back(tmpVert);
			indices.push_back(indexNum);
			indexNum++;
			//vectorの解放
			vectorRelease(faceData);
		}
		//面数が3の場合
		if (f[i].fi[3] == "Nothing") { //面数が5以上の場合もあるので、今回は4つ目にNothingを挿入し、それ以降は無視している
			//何もしない
		}
		//面数が4の場合
		else {
			OBJVertex tmpVert; //OBJvertices一時保管用
			std::vector<std::string > faceData = split(f[i].fi[3], "/");

			switch (faceData.size()) {
			case 1: //頂点データのみ
				tmpVert.pos = v[std::stoi(faceData[0]) - 1]; //objファイルの頂点idはなぜか1スタートなので-1を忘れずに
				tmpVert.uv = XMFLOAT2(0.0f, 0.0f); //デフォルトは0
				tmpVert.normal = XMFLOAT3(0.0f, 0.0f, 0.0f); //デフォルトは0
				break;
			case 2: //頂点/テクスチャ座標
				tmpVert.pos = v[std::stoi(faceData[0]) - 1];
				tmpVert.uv = vt[std::stoi(faceData[1]) - 1];
				tmpVert.normal = XMFLOAT3(0.0f, 0.0f, 0.0f); //デフォルトは0
				break;
			case 3: //頂点//法線 or 頂点/テクスチャ座標/法線
				if (faceData[1].length() == 0) { //頂点//法線
					tmpVert.pos = v[std::stoi(faceData[0]) - 1];
					tmpVert.uv = XMFLOAT2(0.0f, 0.0f);
					tmpVert.normal = vn[std::stoi(faceData[2]) - 1];
				}
				else { //頂点/テクスチャ座標/法線
					tmpVert.pos = v[std::stoi(faceData[0]) - 1];
					tmpVert.uv = vt[std::stoi(faceData[1]) - 1];
					tmpVert.normal = vn[std::stoi(faceData[2]) - 1];
				}
				break;
			}
			OBJvertices.push_back(tmpVert);

			//例えば0,1,2,3の場合
			//.objは頂点が右回りなので, 0,1,2 と0,2,3になる
			indices.push_back(indexNum-3);
			indices.push_back(indexNum-1);
			indices.push_back(indexNum);
			indexNum++;
		}
	}
	std::cout << "データ解析終了 : " << clock() / 1000.0f << "秒\nデータ解析時間 : " << (clock() - time) / 1000 << "秒\n";
	
	vectorRelease(v);
	vectorRelease(vt);
	vectorRelease(vn);
	vectorRelease(f);

	/*-----VertexBufferの生成-----*/
	//HeapPropertyの設定
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	//ResourcDescの定義
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = OBJvertices.size() * sizeof(OBJVertex);
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count = 1;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
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
	std::copy(std::begin(OBJvertices), std::end(OBJvertices), vertMap);
	vertexBuffer->Unmap(0, nullptr);

	/*-----VertexBufferViewの生成-----*/
	vbView = {};
	vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vbView.SizeInBytes = OBJvertices.size() * sizeof(OBJVertex);
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

	/*-----IndexBufferViewの生成　-----*/
	ibView = {};
	ibView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R32_UINT;
	ibView.SizeInBytes = indices.size() * sizeof(indices[0]);

	std::cout << "----------wavefrontOBJ読み込み終了----------\n";

	return S_OK;
}

//指定した文字列splitterで文字列strを分割
std::vector<std::string> Object::split(std::string str, std::string splitter) {
	std::vector<std::string> str_s;
	str_s.reserve(8);
	int offset = 0; //文字列探索開始位置
	int splitPos = 0; //分割点位置

	//最初の分割点探索
	splitPos = str.find(splitter, offset);
	if (splitPos == -1) {
		str_s.push_back(str);
		return str_s; //もし分割点が存在しなかったらこの時点で終了,文字列をそのまま返す
	}
	while (splitPos != -1) {
		str_s.push_back(str.substr(offset, splitPos - offset));

		//オフセット位置と次の分割点を探す
		offset = splitPos + 1; //次の探索に、発見済みの分割点を含まない
		splitPos = str.find(splitter, offset);
	}
	//最後の一回
	str_s.push_back(str.substr(offset));

	return str_s;
}

template<typename T>
void Object::vectorRelease(std::vector<T>& vec) {
	if (vec.data() == nullptr) return; //nullptrなら関数終了
	//最近のvectorの解放(sizeを0にしてからcapacityを0に)の仕方らしい
	vec.clear();
	vec.shrink_to_fit();
}

void Object::Release() {
	vectorRelease(PMDvertices);
	vectorRelease(OBJvertices);
	vectorRelease(indices);

	delete(inputLayout);

	if (vertexBuffer) vertexBuffer->Release();
	if (indexBuffer) indexBuffer->Release();
	if (materialBuffer) materialBuffer->Release();
}