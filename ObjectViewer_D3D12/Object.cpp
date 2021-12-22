#include"Object.h"
#pragma warning(disable: 4996)

OBJFaceData::OBJFaceData(int cnt) {
	vertexCnt = cnt;
	dataIndex = (int**)malloc(sizeof(int*) * cnt);
	for (int i = 0; i < cnt; i++) {
		dataIndex[i] = (int*)malloc(sizeof(int) * 3);
	}
}

OBJFaceData::~OBJFaceData() {
	for (int i = 0; i < vertexCnt - 1; i++) {
		if (dataIndex[i] != NULL) free(dataIndex[i]);
		dataIndex[i] = NULL;
	}
	if (dataIndex != NULL) free(dataIndex);
	dataIndex = NULL;
}

OBJFaceData::OBJFaceData(const OBJFaceData& fd) {
	this->vertexCnt = fd.vertexCnt;

	this->dataIndex = (int**)malloc(sizeof(int*) * this->vertexCnt);
	for (int i = 0; i < this->vertexCnt; i++) {
		this->dataIndex[i] = (int*)malloc(sizeof(int) * 3);
		for (int j = 0; j < 3; j++) {
			dataIndex[i][j] = fd.dataIndex[i][j];
		}
	}
}

OBJTextureBuffers::~OBJTextureBuffers() {
	if (ambTexBuffer) ambTexBuffer->Release();
	if (difTexBuffer) difTexBuffer->Release();
	if (speTexBuffer) speTexBuffer->Release();
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

	texCnt = 0;
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
	if (ObjectLoaded) { //Check this class has already loaded ObjectData
		std::cout << "Object File is already loaded\n";
		return E_FAIL;
	}
	ObjectLoaded = true;

	std::cout << "\n-------------------------\n";
	std::cout << "Loading Model(start)" << std::endl;

	HRESULT hr;
	
	PathController pc; //use for controll path

	/*
	modelPath = ~\ObjectViewer\ObjectViewer_D3D12\Model\OBJ\---
	meshPath = ~\ObjectViewer\ObjectViewer_D3D12\Model\OBJ\---\***.obj
	materialPath = ~\ObjectViewer\ObjectViewer_D3D12\Model\OBJ\---\***.mtl
	modelDirName = ---
	*/
	char modelPath[MAX_PATH_LENGTH];
	char meshPath[MAX_PATH_LENGTH];
	char materialPath[MAX_PATH_LENGTH];
	char modelDirName[64];

	//Create Mesh File Path
	pc.CreatePath(path.c_str(), meshPath);
	//Create Model Folder Path
	pc.RemoveLeafPath(meshPath, modelPath);
	//Get Model Folder name
	pc.GetLeafDirectryName(modelPath, modelDirName, 64);

	float loadTime = clock(); //measure loading time

	/*-----Load Model File-----*/
	FILE* modelFp;
	modelFp = fopen(meshPath, "r");
	if (modelFp == NULL) {
		std::cout << "cannnot open model file\n";
		return E_FAIL;
	}
 	
	std::vector<XMFLOAT3> v; //vertex
	std::vector<XMFLOAT2> vt; //UV
	std::vector<XMFLOAT3> vn; //normal
	std::vector<OBJFaceData> fd; //face

	v.reserve(524288);
	vt.reserve(524288);
	vn.reserve(524288);
	fd.reserve(524288);
	
	v.push_back(XMFLOAT3(0.0f,0.0f,0.0f));
	vt.push_back(XMFLOAT2(0.0f, 0.0f));
	vn.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));

	char lineData[MAX_READ_LINEDATA];
	
	std::vector<std::string> splitData;

	bool isMaterial = false; //if there is no material, this application doesn't load .mtl file

	int idxOffset = 0;

	/*
	data is Line data, so we have to read data by a line and split data with blank
	*/
	while (fgets(lineData, MAX_READ_LINEDATA, modelFp) != NULL) {
		OBJ_splitBlank(lineData, splitData);
		if (splitData[0] == "v") { //if data is vertex
			try {
				v.push_back(XMFLOAT3(std::stof(splitData[1]), std::stof(splitData[2]), std::stof(splitData[3])));
			}
			catch (std::exception& e) {
				v.push_back(XMFLOAT3(0, 0, 0));
			}
		}
		else if (splitData[0] == "vt") { //if data is UV
			try {
				vt.push_back(XMFLOAT2(std::stof(splitData[1]), std::stof(splitData[2])));
			}
			catch (std::exception& e) {
				vt.push_back(XMFLOAT2(0, 0));
			}
		}
		else if (splitData[0] == "vn") { //if data is normal
			try {
				vn.push_back(XMFLOAT3(std::stof(splitData[1]), std::stof(splitData[2]), std::stof(splitData[3])));
			}
			catch (std::exception& e) {
				vn.push_back(XMFLOAT3(0, 0, 0));
			}
		}
		else if (splitData[0] == "f") { //if data is face
			/*
			DirectX draw mesh with Triangle Polygon, but .obj File has 4 more vertices in one polygon.
			Application has to deal with this problem
			*/
			idxOffset += (((splitData.size() - 1) - 2) * 3);

			OBJFaceData tmpData(splitData.size() - 1);
			for (int i = 0; i < tmpData.vertexCnt; i++) {
				OBJ_splitSlash(splitData[i + 1], tmpData.dataIndex[i]);
			}
			fd.push_back(tmpData);
		}
		else if (splitData[0] == "mtllib") { //it means "There is a Material File"
			isMaterial = true; //if flaged, application read .metl file

			for (int i = 1; i < splitData.size() - 1; i++) {
				splitData[1] += " " + splitData[i + 1];
			}
			int slashPos = splitData[1].find_first_of("/");
			if (slashPos != std::string::npos) splitData[1].erase(0, slashPos + 1);


			//Create MaterialPath
			pc.AddLeafPath(modelPath, materialPath, splitData[1].c_str());
		}
		else if (splitData[0] == "usemtl") { //it means "mesh uses this material"
			OBJMaterialRef tmpMatRef;
			for (int splitNum = 1; splitNum < splitData.size(); splitNum++) {
				tmpMatRef.matName += splitData[splitNum];
			}
			tmpMatRef.idxOffset = idxOffset;
			matRef.push_back(tmpMatRef);
		}
		splitData.clear();
	}
	fclose(modelFp);

	//decide how many materials will apply to meshes
	for (int i = 0; i < matRef.size() - 1; i++) {
		matRef[i].idxNum = matRef[i + 1].idxOffset - matRef[i].idxOffset;
	}
	matRef[matRef.size() - 1].idxNum = idxOffset - matRef[matRef.size() - 1].idxOffset;

	if (isMaterial) {
		FILE* matFp;
		matFp = fopen(materialPath, "r");
		if (matFp == NULL) {
			std::cout << "cannnot open material file\n";
			isMaterial = false;
			return E_FAIL;
		}

		else {
			/*
			Material Data is defined frome "newmtl" to next blank line
			*/
			int materialNum = 0;
			OBJMaterial tmpMat;
			std::string tmpName;

			while (fgets(lineData, MAX_READ_LINEDATA, matFp) != NULL) {
				OBJ_splitBlank(lineData, splitData);
				if (splitData[0] == "newmtl") {

					if (materialNum > 0) materials.push_back(tmpMat);
					//push this tmpData to MaterialData, so application should reset tmpData every loop
					tmpMat.Init();
					tmpName.clear();
					/*
					program split data with blank, but material cen be named with blank, so program have to combine each name data
					*/
					for (int splitNum = 1; splitNum < splitData.size(); splitNum++) {
						tmpName += splitData[splitNum];
					}
					tmpMat.materialName = tmpName;

					/*
					this data will be referenced by Drawing process
					*/
					for (int i = 0; i < matRef.size(); i++) {
						if (matRef[i].matName == tmpName) {
							matRef[i].matID = materialNum;
						}
					}
					materialNum++;
				}
				else {
					if (splitData[0] == "Ka") { //if ambient color
						tmpMat.mcb.ambient = XMFLOAT4(std::stof(splitData[1]), std::stof(splitData[2]), std::stof(splitData[3]), 1.0f);
					}
					else if (splitData[0] == "Kd") { //if diffuse color
						tmpMat.mcb.diffuse = XMFLOAT4(std::stof(splitData[1]), std::stof(splitData[2]), std::stof(splitData[3]), 1.0f);
					}
					else if (splitData[0] == "Ks") { //if specular color
						tmpMat.mcb.specular = XMFLOAT4(std::stof(splitData[1]), std::stof(splitData[2]), std::stof(splitData[3]), 1.0f);
					}
					else if (splitData[0] == "Ns") { //if specular power
						tmpMat.mcb.Nspecular = std::stof(splitData[1]);
					}


					//Texture Path
					/*
					Create TexturePath
					Get the TexturePath from .obj File, but this path is not Loadable for this program, so we should Convert TexturePath to Loadable Path.
					*/
					else if (splitData[0] == "map_Ka") { //if ambient texture
						for (int i = 1; i < splitData.size() - 1; i++) {
							splitData[1] += " " + splitData[i + 1];
						}
						int check = pc.PathFinder(splitData[1].c_str(), tmpMat.ambTexPath, modelPath);
						if (check == -1) {
							std::cout << "Error : there is no Texture File" << std::endl;
						}
						tmpMat.texCnt++;
					}
					else if (splitData[0] == "map_Kd") { //if diffuse texture
						for (int i = 1; i < splitData.size() - 1; i++) {
							splitData[1] += " " + splitData[i + 1];
						}
						int check = pc.PathFinder(splitData[1].c_str(), tmpMat.difTexPath, modelPath);
						if (check == -1) {
							std::cout << "Error : there is no Texture File" << std::endl;
						}
						tmpMat.texCnt++;
					}
					else if (splitData[0] == "map_Ks") { //specular Texture
						for (int i = 1; i < splitData.size() - 1; i++) {
							splitData[1] += " " + splitData[i + 1];
						}
						int check = pc.PathFinder(splitData[1].c_str(), tmpMat.speTexPath, modelPath);
						if (check == -1) {
							std::cout << "Error : there is no Texture File" << std::endl;
						}
						tmpMat.texCnt++;
					}
				}
			}
			materials.push_back(tmpMat);

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
	}

	unsigned indexNum = 0;
	for (int i = 0; i < fd.size(); i++) {
		/*
		face data is some format.
		x// -> vertex only
		x/y -> vertex and UV
		x//y -> vertex and normal
		x/y/z -> vertex and UV and normal
		*/

		/*
		face data is define one face in one line.
		*/
		for (int j = 0; j < fd[i].vertexCnt; j++) {
			OBJVertex tmpVert;

			tmpVert.pos = v[fd[i].dataIndex[j][0]];
			tmpVert.uv = vt[fd[i].dataIndex[j][1]];
			tmpVert.normal = vn[fd[i].dataIndex[j][2]];
				
			
			vertices.push_back(tmpVert);
		}

		//Create index
		/*
		if one face has 3 vertices, this face is Triangle polygon, so push index in order
		if one face has 4 more vertices, program should create (vertices count - 2) Triangles.
		*/
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

	//Set HeapProperty
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	//describe Resource(now for VertexBuffer)
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
	/*-----CreateVertexBuffer-----*/
	hr = device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexBuffer));
	if (FAILED(hr)) {
		std::cout << "Failed to Create vertexBuffer\n";
		return hr;
	}
	/*-----Map to Vertices Data-----*/
	OBJVertex* vertMap = nullptr;
	hr = vertexBuffer->Map(0, nullptr, (void**)&vertMap);
	if (FAILED(hr)) {
		std::cout << "Failed to Map vertex\n";
		return hr;
	}
	std::copy(std::begin(vertices), std::end(vertices), vertMap);
	vertexBuffer->Unmap(0, nullptr);

	/*-----Create VertexBufferView-----*/
	vbView = {};
	vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vbView.SizeInBytes = vertexBuffer->GetDesc().Width;
	vbView.StrideInBytes = sizeof(OBJVertex);

	/*-----Create IndexBuffer-----*/
	//Change ResourceDesc's Width
	resDesc.Width = indices.size() * sizeof(unsigned);
	//Create IndexBuffer
	hr = device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&indexBuffer));
	if (FAILED(hr)) {
		std::cout << "Failed to Create indexBuffer\n";
		return hr;
	}
	/*-----Map to Indices Data-----*/
	unsigned* idxMap = nullptr;
	hr = indexBuffer->Map(0, nullptr, (void**)&idxMap);
	if (FAILED(hr)) {
		std::cout << "Failed to Map index\n";
		return hr;
	}
	std::copy(std::begin(indices), std::end(indices), idxMap);
	indexBuffer->Unmap(0, nullptr);

	/*-----Create IndexBufferView-----*/
	ibView = {};
	ibView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R32_UINT;
	ibView.SizeInBytes = indices.size() * sizeof(indices[0]);

	/*-----Create MaterialBuffer(this buffer create as ConstantBuffer, so this process needs to create Descriptor)-----*/
	//Change ResourceDesc
	if (isMaterial) resDesc.Width = sizeof(OBJMaterialCB) * materials.size();
	else resDesc.Width = 1;
	//CreateMaterialBuffer
	hr = device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&materialBuffer));
	if (FAILED(hr)) {
		std::cout << "Failed to Create materialBuffer" << std::endl;
	}
	/*-----Map to MaterialData-----*/
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

	/*-----Create DescriptorHeap for Material-----*/
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
	//Create MaterialBufferView(GPU need to know Buffer Adress, Create ViewCount MaterialBufferViews)
	D3D12_CONSTANT_BUFFER_VIEW_DESC matCBVDesc = { };
	matCBVDesc.BufferLocation = materialBuffer->GetGPUVirtualAddress();
	matCBVDesc.SizeInBytes = sizeof(OBJMaterialCB);
	D3D12_CPU_DESCRIPTOR_HANDLE handleOffset = materialDescHeap->GetCPUDescriptorHandleForHeapStart();
	for (int i = 0; i < materials.size(); i++) {
		device->CreateConstantBufferView(&matCBVDesc, handleOffset);
		handleOffset.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		matCBVDesc.BufferLocation += sizeof(OBJMaterialCB);
	}
	/*-----LoadTexture and CreateTextureBuffer-----*/
	//Create DescriptorHeap
	D3D12_DESCRIPTOR_HEAP_DESC texHeapDesc = {};
	texHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	texHeapDesc.NodeMask = 0;
	texHeapDesc.NumDescriptors = materials.size() * 3; //every materials has three texture(amb,dif,spe)
	texHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	hr = device->CreateDescriptorHeap(&texHeapDesc, IID_PPV_ARGS(&textureDescHeap));
	if (FAILED(hr)) {
		std::cout << "Error : Cannnot Create DescriptorHeap" << std::endl;
		return hr;
	}
	texBuffers.resize(materials.size());
	for (int i = 0; i < materials.size(); i++) {
		wchar_t path[MAX_PATH_LENGTH];
		mbstowcs(path, materials[i].ambTexPath, MAX_PATH_LENGTH);
		hr = OBJ_CreateTextureBuffer(path, MAX_PATH_LENGTH, i, 0, device);
		mbstowcs(path, materials[i].difTexPath, MAX_PATH_LENGTH);
		//std::cout << materials[i].difTexPath << std::endl;
		hr = OBJ_CreateTextureBuffer(path, MAX_PATH_LENGTH, i, 1, device);
		mbstowcs(path, materials[i].speTexPath, MAX_PATH_LENGTH);
		hr = OBJ_CreateTextureBuffer(path, MAX_PATH_LENGTH, i, 2, device);
		//std::cout << materials[i].speTexPath << std::endl;

		//Create shaderResourceView
		/*
		ShaderResourceView must be Created whether Texture load completed or not
		*/
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		D3D12_CPU_DESCRIPTOR_HANDLE heapH = textureDescHeap->GetCPUDescriptorHandleForHeapStart();
		heapH.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * (3 * i); //StartPosition
		//Create 3 Descriptor
		device->CreateShaderResourceView(texBuffers[i].ambTexBuffer, &srvDesc, heapH);
		heapH.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		device->CreateShaderResourceView(texBuffers[i].difTexBuffer, &srvDesc, heapH);
		heapH.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		device->CreateShaderResourceView(texBuffers[i].speTexBuffer, &srvDesc, heapH);
	}

	std::cout << "Loading Model is finished(" << (clock() - loadTime) / 1000 << "sec)" << std::endl;
	
	std::cout << "-------------------------\n";

	return S_OK;
}

HRESULT Object::OBJ_CreateTextureBuffer(const wchar_t* pathName, size_t pathLength, int materialNum, int textureNum, ID3D12Device* device) {
	HRESULT hr;
	
	ScratchImage sImg = {};
	TexMetadata md = {};

	hr = LoadFromWICFile(pathName, WIC_FLAGS_NONE, &md, sImg);
	if (SUCCEEDED(hr)) {
		const Image* img = sImg.GetImage(0, 0, 0);

		/*-----Create TextureBuffer-----*/
		//Set HeapProperty
		D3D12_HEAP_PROPERTIES heapProp = {};
		heapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
		heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
		heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
		heapProp.CreationNodeMask = 0;
		heapProp.VisibleNodeMask = 0;
		//describe Resouce
		D3D12_RESOURCE_DESC resDesc = {};
		resDesc.Format = md.format;
		resDesc.Width = md.width;
		resDesc.Height = md.height;
		resDesc.DepthOrArraySize = md.arraySize;
		resDesc.SampleDesc.Count = 1;
		resDesc.SampleDesc.Quality = 0;
		resDesc.MipLevels = md.mipLevels;
		resDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(md.dimension);
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		switch (textureNum) { //Create Texture Buffer and Write Data to Buffer
			/*
			CAUTION :
			this method what will be used in this process is not recommended, but I use this method because i have no time
			*/
		case 0:
			hr = device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&texBuffers[materialNum].ambTexBuffer));
			if (FAILED(hr)) std::cout << "Error : Failed to Create TextureBuffer" << std::endl;
			else {
				hr = texBuffers[materialNum].ambTexBuffer->WriteToSubresource(0, nullptr, img->pixels, img->rowPitch, img->slicePitch);
			}
			break;
		case 1:
			hr = device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&texBuffers[materialNum].difTexBuffer));
			if (FAILED(hr)) std::cout << "Error : Failed to Create TextureBuffer" << std::endl;
			else {
				hr = texBuffers[materialNum].difTexBuffer->WriteToSubresource(0, nullptr, img->pixels, img->rowPitch, img->slicePitch);
			}
			break;
		case 2:
			hr = device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&texBuffers[materialNum].speTexBuffer));
			if (FAILED(hr)) std::cout << "Error : Failed to Create TextureBuffer" << std::endl;
			else {
				hr = texBuffers[materialNum].speTexBuffer->WriteToSubresource(0, nullptr, img->pixels, img->rowPitch, img->slicePitch);
			}
			break;
		}
	}
	
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

	std::string splitter = " ";
	std::string tmp;
	int offset = 0;
	int splitPos = 0;

	splitPos = str.find(splitter, offset);
	while (splitPos != -1) { //Loop while next SplitPos doesn't find
		tmp = str.substr(offset, splitPos - offset);

		if (tmp.size() != 0) data.push_back(tmp); //if there is no data between blank and next blank, don't push to vector

		offset = splitPos + 1;
		splitPos = str.find(splitter, offset);
	}
	/*
	if program find last blank,  we should care handling last Data.
	last data is...
	[blank] x/x/x [blank] \n
	or
	[blank] x/x/x \n
	*/
	if (str.substr(offset).size() != 0 && str.substr(offset) != "\n")  data.push_back(str.substr(offset, (str.size() - offset - 1)));

	if (data.size() == 0) data.push_back("\0"); //if there is no element in vector, program is clashed by Exceptinal Error
}

void Object::OBJ_splitSlash(std::string str, int* data) {
	std::string splitter = "/";
	std::string tmp;

	int offset = 0;
	int splitPos = 0;

	splitPos = str.find(splitter, offset);

	int i = 0;
	while (splitPos != std::string::npos) {
		tmp = str.substr(offset, splitPos - offset);
		if (tmp.size() == 0) data[i] = 0; //if there is no data between split and next split, push 0 to vector
		else data[i] = std::stoi(tmp);

		offset = splitPos + 1;
		splitPos = str.find(splitter, offset);
		
		i++;
	}

	
	if (i == 1) { //this case is 1 time split. it means data must be "vertex / UV"
		data[1] = std::stoi((str.substr(offset)));
		data[2] = 0;
	}
	else if (i == 2) { //this case is 2 times split. it means this data is "vertex / UV / Normal" or "vertex // Normal"
		if (str.substr(offset).size() == 0) data[2] = 0;
		else data[2] = std::stoi((str.substr(offset)));
	}
}

/*
this function return MaterialReference Index which match materialName.
if do not match materialName, return value is -1
*/
int Object::findMaterialIndex(std::vector<OBJMaterialRef> mr, std::string material) {
	for (int i = 0; i < mr.size(); i++) {
		if (mr[i].matName == material) return i;
	}

	return -1; 
}

HRESULT Object::DrawObjet(ID3D12GraphicsCommandList* cmdList, ID3D12PipelineState *pipelineState, ID3D12RootSignature* rootsignature, Camera camera, D3D12_CPU_DESCRIPTOR_HANDLE rtvH, D3D12_CPU_DESCRIPTOR_HANDLE dsvH, D3D12_VIEWPORT vp, D3D12_RECT rc) {
	/*
	//グラフィクスパイプラインのセット
	cmdList->SetPipelineState(pipelineState);

	//レンダーターゲットのセット
	cmdList->OMSetRenderTargets(1, &rtvH, false, &dsvH);

	//デプス/ステンシルバッファのセット
	cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	//ビューポートとシザー矩形のセット
	cmdList->RSSetViewports(1, &vp);
	cmdList->RSSetScissorRects(1, &rc);

	//ルートシグネチャの設定
	cmdList->SetGraphicsRootSignature(rootsignature);

	D3D12_GPU_DESCRIPTOR_HANDLE heapHCBV, heapHMat;

	heapHCBV = m_cbvHeap->GetGPUDescriptorHandleForHeapStart();

	m_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (int objCnt = 0; objCnt < m_LoadedObjCount; objCnt++) {

		D3D12_GPU_DESCRIPTOR_HANDLE tmpHandle;
		UpdateWorldMatrix(m_objs[objCnt], objCnt);
		UpdateViewMatrix(camera, objCnt);

		//Switch ConstantBuffer for Each Objects
		m_cmdList->SetDescriptorHeaps(1, &m_cbvHeap);
		tmpHandle.ptr = heapHCBV.ptr + m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * objCnt;
		m_cmdList->SetGraphicsRootDescriptorTable(0, tmpHandle);

		m_cmdList->IASetVertexBuffers(0, 1, &m_objs[objCnt].vbView);
		m_cmdList->IASetIndexBuffer(&m_objs[objCnt].ibView);

		//Switch MaterialBuffer and TextureBuffer for Each Vertices
		heapHMat = m_objs[objCnt].materialDescHeap->GetGPUDescriptorHandleForHeapStart();
		for (int i = 0; i < m_objs[objCnt].matRef.size(); i++) {
			//set Texture
			m_cmdList->SetDescriptorHeaps(1, &m_objs[objCnt].textureDescHeap); //texture
			D3D12_GPU_DESCRIPTOR_HANDLE heapH = m_objs[objCnt].textureDescHeap->GetGPUDescriptorHandleForHeapStart();
			heapH.ptr += m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * (3 * m_objs[objCnt].matRef[i].matID);
			m_cmdList->SetGraphicsRootDescriptorTable(2, heapH);
			//set Material
			m_cmdList->SetDescriptorHeaps(1, &m_objs[objCnt].materialDescHeap); //material
			tmpHandle.ptr = heapHMat.ptr + m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * m_objs[objCnt].matRef[i].matID;
			m_cmdList->SetGraphicsRootDescriptorTable(1, tmpHandle);
			m_cmdList->DrawIndexedInstanced(m_objs[objCnt].matRef[i].idxNum, 1, m_objs[objCnt].matRef[i].idxOffset, 0, 0);
		}
	}*/

	return S_OK;
}

template<typename T>
void Object::vectorRelease(std::vector<T>& vec) {
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
	if (textureDescHeap)textureDescHeap->Release();
}