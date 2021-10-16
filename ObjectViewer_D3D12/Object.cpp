#include"Object.h"
#pragma warning(disable: 4996)

OBJFaceData::OBJFaceData(int cnt) {
	vertexCnt = cnt;
	//�ʂ��\�����钸�_�� * 3�̓�d�z���p�ӂ���
	dataIndex = (int**)malloc(sizeof(int*) * cnt);
	for (int i = 0; i < cnt; i++) {
		dataIndex[i] = (int*)malloc(sizeof(int) * 3);
	}
}

OBJFaceData::~OBJFaceData() {
	for (int i = 0; i < vertexCnt - 1; i++) {
		if (dataIndex[i] != NULL) free(dataIndex[i]);
		dataIndex[i] = NULL; //��d����΍�
	}
	if (dataIndex != NULL) free(dataIndex);
	dataIndex = NULL; //��d����΍�
}

OBJFaceData::OBJFaceData(const OBJFaceData& fd) {
	this->vertexCnt = fd.vertexCnt;

	//�����ƃR�s�[�̓R�s�[�ŕʂ̃��������m�ۂ��Ă����Ȃ��ƍ���
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
	//�I�u�W�F�N�g�̑��d���[�h��h��
	if (ObjectLoaded) {
		std::cout << "Object File is already loaded\n";
		return E_FAIL;
	}
	ObjectLoaded = true;
	std::cout << "\n-------------------------\n";
	std::cout << "Loading Model(start)" << std::endl;

	HRESULT hr;
	
	PathController pc; //�p�X�R���g���[��

	char modelPath[MAX_PATH_LENGTH]; //���f���t�@�C���p�X
	char meshPath[MAX_PATH_LENGTH]; //���b�V���t�@�C���p�X
	char materialPath[MAX_PATH_LENGTH]; //�}�e���A���t�@�C���p�X
	char modelDirName[64]; //���f���t�@�C����(���b�V���t�@�C���p�X, �e�N�X�`���t�@�C���p�X�̐����Ɏg��)

	//���b�V���t�@�C���p�X�̐���
	pc.CreatePath(path.c_str(), meshPath);
	//���f���t�@�C���̃p�X�̐���(���b�V���t�@�C���p�X�̐e�f�B���N�g�������f���t�@�C���Ȃ̂ŁA���b�V���t�@�C���p�X���烁�b�V���t�@�C����.�g���q���폜����)
	pc.RemoveLeafPath(meshPath, modelPath);
	//���f���t�@�C�����̎擾
	pc.GetLeafDirectryName(modelPath, modelDirName, 64);

	/*std::cout << modelPath << std::endl;
	std::cout << meshPath << std::endl;
	std::cout << modelDirName << std::endl;*/

	float loadTime = clock(); //���[�h�J�n���Ԃ��擾

	//�t�@�C�����[�h
	FILE* modelFp; //���f���t�@�C��
	modelFp = fopen(meshPath, "r");
	if (modelFp == NULL) {
		std::cout << "cannnot open model file\n";
		return E_FAIL;
	}
 	
	//��s���ǂݍ��݁A[v],[vt],[vn],[f]���擾
	std::vector<XMFLOAT3> v; //���_���W
	std::vector<XMFLOAT2> vt; //�e�N�X�`�����W
	std::vector<XMFLOAT3> vn; //�@�����
	std::vector<OBJFaceData> fd; //�ʃf�[�^

	//capacity�����̎��_�œ��I�m�ۂ��Ă����A�]���ȃ������̍Ċm�ۂ�h��
	v.reserve(524288);
	vt.reserve(524288);
	vn.reserve(524288);
	fd.reserve(524288);
	
	/*
	OBJ�t�@�C���͖ʃf�[�^�̒��_�J�E���g��1�X�^�[�g�ōs��
	�v�f[0]�Ƀf�t�H���g�l�������ċN���A�ʃf�[�^�̑Ή����Ȃ��Ƃ�(v//�Ƃ�v/vt/�Ƃ�)�́A���_�ɂ��̒l��������
	*/
	v.push_back(XMFLOAT3(0.0f,0.0f,0.0f));
	vt.push_back(XMFLOAT2(0.0f, 0.0f));
	vn.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));

	char lineData[MAX_READ_LINEDATA]; //1�s�f�[�^
	
	std::vector<std::string> splitData; //�����㕶����i�[�z��
	splitData.reserve(50); //�������Ċm�ۖh�~

	bool isMaterial = false; //�}�e���A���t�@�C�������݂��邩�t���O

	int idxOffset = 0; //�C���f�b�N�X�I�t�Z�b�g 3���_�̎O�p�`�|���S�����b�V���ŕ`�悵�Ă����̂ŁA���_�I�t�Z�b�g�� (�ʂŐ錾����钸�_�� - 2) * 3�X�g���C�h����K�v����

	//1�s��������Ŏ擾�A�������ϊ����f�[�^�Ƃ��ċL�^���Ă���
	while (fgets(lineData, MAX_READ_LINEDATA, modelFp) != NULL) {
		//�ǂݍ��񂾈�s�̃f�[�^�𕪊����A�z��Ɋi�[�ł���`�ɂ���
		OBJ_splitBlank(lineData, splitData);
		if (splitData[0] == "v") { //���_
			v.push_back(XMFLOAT3(std::stof(splitData[1]), std::stof(splitData[2]), std::stof(splitData[3])));
		}
		else if (splitData[0] == "vt") { //UV
			vt.push_back(XMFLOAT2(std::stof(splitData[1]), std::stof(splitData[2])));
		}
		else if (splitData[0] == "vn") { //�@��
			vn.push_back(XMFLOAT3(std::stof(splitData[1]), std::stof(splitData[2]), std::stof(splitData[3])));
		}
		else if (splitData[0] == "f") { //�ʃf�[�^
			idxOffset += (((splitData.size() - 1) - 2) * 3);

			OBJFaceData tmpData(splitData.size() - 1);
			for (int i = 0; i < tmpData.vertexCnt; i++) {
				OBJ_splitSlash(splitData[i + 1], tmpData.dataIndex[i]); //�ʃf�[�^�̓X���b�V����؂�
			}
			fd.push_back(tmpData);
		}
		else if (splitData[0] == "mtllib") { //�}�e���A���t�@�C���p�X
			isMaterial = true; //�}�e���A�����݃t���O�𗧂Ă�
			//�}�e���A���t�@�C�����̋L�q��./����n�߂Ă���ꍇ�͂�����폜����
			int slashPos = splitData[1].find_first_of("/");
			if (slashPos != std::string::npos) splitData[1].erase(0, slashPos + 1);

			pc.AddLeafPath(modelPath, materialPath, splitData[1].c_str()); //���f���t�@�C���p�X�Ƀ}�e���A���t�@�C���p�X��t������
			std::cout << materialPath << std::endl;
		}
		else if (splitData[0] == "usemtl") { //�}�e���A����
			OBJMaterialRef tmpMatRef;
			for (int splitNum = 1; splitNum < splitData.size(); splitNum++) { //�}�e���A�������󔒍��݂Œ�`����Ă���ꍇ������̂őΉ�
				tmpMatRef.matName += splitData[splitNum];
			}
			tmpMatRef.idxOffset = idxOffset;
			matRef.push_back(tmpMatRef);
		}
		splitData.clear();
	}
	fclose(modelFp);
	
	//�}�e���A���K�p�������߂�
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

	if (isMaterial) { //���̌�̏����̓}�e���A���t�@�C�����������ꍇ�ɂ̂ݍs��
		//�t�@�C�����[�h
		FILE* matFp; //�}�e���A���t�@�C��
		matFp = fopen(materialPath, "r");
		if (matFp == NULL) {
			std::cout << "cannnot open material file\n";
		}

		//std::cout << materialPath << std::endl;

		/*
		�}�e���A���t�@�C���́Anewmtl���玟�̋�s�܂�
		*/
		int materialNum = 0;
		OBJMaterial tmpMat;
		std::string tmpName;
		while (fgets(lineData, MAX_READ_LINEDATA, matFp) != NULL) {
			OBJ_splitBlank(lineData, splitData);
			if (splitData[0] == "newmtl") { //�}�e���A���t�@�C���ł̃}�e���A���錾
				if (materialNum > 0) materials.push_back(tmpMat);
				tmpMat.Init();
				tmpName.clear();
				for (int splitNum = 1; splitNum < splitData.size(); splitNum++) {
					tmpName += splitData[splitNum];
				}
				tmpMat.materialName = tmpName;
				//�}�e���A���Q�ƃf�[�^�Ǝ��ۂ̃}�e���A���̑Ή��t�����s��
				for (int i = 0; i < matRef.size(); i++) {
					if (matRef[i].matName == tmpName) {
						matRef[i].matID = materialNum;
					}
				}
				materialNum++;
			}
			else {
				if (splitData[0] == "Ka") { //�A���r�G���g�F
					tmpMat.mcb.ambient = XMFLOAT4(std::stof(splitData[1]), std::stof(splitData[2]), std::stof(splitData[3]), 1.0f);
				}
				else if (splitData[0] == "Kd") { //�f�B�t���[�Y�F
					tmpMat.mcb.diffuse = XMFLOAT4(std::stof(splitData[1]), std::stof(splitData[2]), std::stof(splitData[3]), 1.0f);
				}
				else if (splitData[0] == "Ks") { //�X�y�L�����F
					tmpMat.mcb.specular = XMFLOAT4(std::stof(splitData[1]), std::stof(splitData[2]), std::stof(splitData[3]), 1.0f);
				}
				else if (splitData[0] == "Ns") { //�X�y�L�����w��
					tmpMat.mcb.Nspecular = std::stof(splitData[1]);
				}


				//�t�@�C���p�X��K�؂Ȍ`�ɕϊ�����K�v����
				/*
				1.�܂����f���t�@�C���f�B���N�g�����Ō���
				2.���������ꍇ�́A���̃t�@�C���f�B���N�g���ȉ��̃p�X���擾���AmodelPath�Ɍ�������
				3.������Ȃ��ꍇ�́A�t�H���_�������ꂸ�ɒ��ڃ��f���t�@�C�������ɒu����Ă���Ƃ݂Ȃ��āA����������̂܂�modelPath�Ɍ�������
				*/
				else if (splitData[0] == "map_Ka") { //�A���r�G���g�e�N�X�`��
					std::string tmpMatPath;
					for (int i = 1; i < splitData.size(); i++) { //����̌����͋󔒂��󔒂Ƃ��Ĉ���
						tmpMatPath += splitData[i];
						tmpMatPath += " ";
					}
					tmpMatPath.erase(tmpMatPath.end() - 1); //�Ō�̋󔒂͗]�v�Ȃ̂ō폜

					char tmpStr[MAX_PATH_LENGTH];
					pc.GetAfterPathFromDirectoryName(tmpMatPath.c_str(), tmpStr, modelDirName);
					pc.AddLeafPath(modelPath, tmpMat.ambTexPath, tmpStr);
				}
				else if (splitData[0] == "map_Kd") { //�f�B�t���[�Y�e�N�X�`��
					std::string tmpMatPath;
					for (int i = 1; i < splitData.size(); i++) { //����̌����͋󔒂��󔒂Ƃ��Ĉ���
						tmpMatPath += splitData[i];
						tmpMatPath += " ";
					}
					tmpMatPath.erase(tmpMatPath.end() - 1); //�Ō�̋󔒂͗]�v�Ȃ̂ō폜

					char tmpStr[MAX_PATH_LENGTH];
					pc.GetAfterPathFromDirectoryName(tmpMatPath.c_str(), tmpStr, modelDirName);
					pc.AddLeafPath(modelPath, tmpMat.difTexPath, tmpStr);
				}
				else if (splitData[0] == "map_Ks") { //�X�y�L�����e�N�X�`��
					std::string tmpMatPath;
					for (int i = 1; i < splitData.size(); i++) { //����̌����͋󔒂��󔒂Ƃ��Ĉ���
						tmpMatPath += splitData[i];
						tmpMatPath += " ";
					}
					tmpMatPath.erase(tmpMatPath.end() - 1); //�Ō�̋󔒂͗]�v�Ȃ̂ō폜
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

	unsigned indexNum = 0; //�C���f�b�N�X�� 1���_�������邽�тɃC���N�������g����
	//�ʏ������ƂɁA�@���_���̊i�[�ƒ��_�C���f�b�N�X�̐���;
	for (int i = 0; i < fd.size(); i++) {
		//���̎��_�Ń}�e���A���Q�ƃf�[�^�̃C���f�b�N�X�I�t�Z�b�g�͖ʂ̔ԍ��ŋL�^����Ă���̂ŁA�����ŃC���f�b�N�X�ԍ��ɏC������
		

		/*
		"/"�Ȃ��ꍇ : ���_�̂�
		"a/b"�̏ꍇ : ���_/�e�N�X�`�����W
		"a//b"�̏ꍇ : ���_//�@��
		"a/b/c"�̏ꍇ : ���_/�e�N�X�`�����W/�@��
		*/

		//�܂��ʏ��𕪊�����
		/*
		obj�t�@�C���́A�ʏ��(f)�ɒ��_�ƒ��_�ɑΉ�����UV���W����і@���̏�񂪂���
	    f�̈�v�f���꒸�_�ƂȂ�
		*/

		int splitData[3];
		for (int j = 0; j < fd[i].vertexCnt; j++) {
			OBJVertex tmpVert; //OBJvertices�ꎞ�ۊǗp

			tmpVert.pos = v[fd[i].dataIndex[j][0]];
			tmpVert.uv = vt[fd[i].dataIndex[j][1]];
			tmpVert.normal = vn[fd[i].dataIndex[j][2]];
				
			//���_�f�[�^�փv�b�V��
			vertices.push_back(tmpVert);
		}

		//f[i].faceNum�̌������ƂɃC���f�b�N�X�l�𐶐�
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

	//HeapProperty�̐ݒ�
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	//ResourcDesc�̒�`
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
	/*-----VertexBuffer�̐���-----*/
	//VertexBuffer(Resource)�̐���
	hr = device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexBuffer));
	if (FAILED(hr)) {
		std::cout << "Failed to Create vertexBuffer\n";
		return hr;
	}
	/*-----���_�f�[�^�̃}�b�v-----*/
	OBJVertex* vertMap = nullptr;
	hr = vertexBuffer->Map(0, nullptr, (void**)&vertMap);
	if (FAILED(hr)) {
		std::cout << "Failed to Map vertex\n";
		return hr;
	}
	std::copy(std::begin(vertices), std::end(vertices), vertMap);
	vertexBuffer->Unmap(0, nullptr);

	/*-----VertexBufferView�̐���-----*/
	vbView = {};
	vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vbView.SizeInBytes = vertices.size() * sizeof(OBJVertex);
	vbView.StrideInBytes = sizeof(OBJVertex);

	/*-----IndexBuffer�̐���-----*/
	//ResourceDesc�̕ύX
	resDesc.Width = indices.size() * sizeof(indices[0]);
	//IndexBuffer�̐���
	hr = device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&indexBuffer));
	if (FAILED(hr)) {
		std::cout << "Failed to Create indexBuffer\n";
		return hr;
	}
	/*-----�C���f�b�N�X�f�[�^�̃}�b�v-----*/
	unsigned* idxMap = nullptr;
	hr = indexBuffer->Map(0, nullptr, (void**)&idxMap);
	if (FAILED(hr)) {
		std::cout << "Failed to Map index\n";
		return hr;
	}
	std::copy(std::begin(indices), std::end(indices), idxMap);
	indexBuffer->Unmap(0, nullptr);

	/*-----IndexBufferView�̐���-----*/
	ibView = {};
	ibView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R32_UINT;
	ibView.SizeInBytes = indices.size() * sizeof(indices[0]);

	/*-----MaterialBuffer�̐���-----*/
	//ResourceDesc�̕ύX
	resDesc.Width = sizeof(OBJMaterialCB) * materials.size();
	//MaterialBuffer�̐���
	hr = device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&materialBuffer));
	if (FAILED(hr)) {
		std::cout << "Failed to Create materialBuffer" << std::endl;
	}
	std::cout << materialBuffer->GetDesc().Width << std::endl;
	/*-----�}�e���A���f�[�^�̃}�b�v-----*/
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

	/*-----�}�e���A���p�f�B�X�N���v�^�q�[�v�̐���-----*/
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
	//�}�e���A���o�b�t�@�r���[�����(�o�b�t�@�̃A�h���X(�ʒu)�������Ȃ��Ⴂ���Ȃ��̂ŁA�}�e���A�������̃r���[���K�v�炵��)
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

	std::string splitter = " "; //����������
	std::string tmp; //�ꎞ�i�[�p������

	int offset = 0;
	int splitPos = 0;

	splitPos = str.find(splitter, offset);
	while (splitPos != -1) {
		tmp = str.substr(offset, splitPos - offset);

		if (tmp.size() != 0) data.push_back(tmp); //�󔒂Ƌ󔒂̊Ԃɕ����񂪂Ȃ��ꍇ�ɂ̓v�b�V�����Ȃ�

		offset = splitPos + 1; //�T���J�n�_��i�߂�
		splitPos = str.find(splitter, offset);
	}

	if (str.substr(offset).size() != 0) data.push_back(str.substr(offset, (str.size() - offset - 1))); //�Ō�̈��(�G�X�P�[�v�V�[�P���X\n������̂Ŗ������邽�߂�1�v�f�Ȃ�)
}

void Object::OBJ_splitSlash(std::string str, int* data) {
	std::string splitter = "/"; //����������
	std::string tmp; //�ꎞ�i�[�p������

	int offset = 0;
	int splitPos = 0;

	splitPos = str.find(splitter, offset);
	for(int i = 0;i < 2;i++){
		tmp = str.substr(offset, splitPos - offset);

		if (tmp.size() == 0) data[i] = 0; //�X���b�V���ƃX���b�V���̊Ԃɕ������Ȃ��ꍇ�́A0���v�b�V������
		else data[i] = std::stoi(tmp);

		offset = splitPos + 1; //�T���J�n�_��i�߂�
		splitPos = str.find(splitter, offset);
	}

	if (str.substr(offset).size() == 0) data[2] = 0; //�Ō�̈��
	else data[2] = std::stoi((str.substr(offset)));
}

int Object::findMaterialIndex(std::vector<OBJMaterialRef> mr, std::string material) {
	for (int i = 0; i < mr.size(); i++) {
		if (mr[i].matName == material) return i;
	}

	return -1; //�܂����Q�Ƃ̃}�e���A���ł���ꍇ��-1��Ԃ�
}



template<typename T>
void Object::vectorRelease(std::vector<T>& vec) {
	//�ŋ߂�vector�̉��(size��0�ɂ��Ă���capacity��0��)�̎d���炵��
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