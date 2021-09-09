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
	ObjectLoaded = true; //�I�u�W�F�N�g�t�@�C���𑽏d���[�h���Ȃ�
	HRESULT hr;

	/*-----�w�b�_���̎擾-----*/
	PMDHeader pmdheader;
	char signature[3] = {};
	std::string strModelPath = "Model/" + ModelName + ".pmd";
	auto fp = fopen(strModelPath.c_str(), "rb");
	fread(signature, sizeof(signature), 1, fp);
	fread(&pmdheader, sizeof(pmdheader), 1, fp);
	std::cout << "�ǂݍ��݃��f�����\n" << "�o�[�W���� : " << pmdheader.version << "\n" << "���f���� : " << pmdheader.model_name << "\n" << "�R�����g : " << pmdheader.comment << "\n";

	/*-----���_���̊m�F-----*/
	unsigned int vertNum;
	fread(&vertNum, sizeof(vertNum), 1, fp);
	std::cout << "���f�����_�� : " << vertNum << "\n";

	/*-----���_�f�[�^�̓ǂݍ���-----*/
	PMDvertices.resize(vertNum * pmdvertex_size);
	fread(PMDvertices.data(), PMDvertices.size(), 1, fp);

	/*-----VertexBuffer�̐���-----*/
	//HeapProperty�̐ݒ�
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	//ResourcDesc�̒�`
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
	//VertexBuffer(Resource)�̐���
	hr = device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexBuffer));
	if (FAILED(hr)) {
		std::cout << "Failed to Create vertexBuffer\n";
		return hr;
	}
	/*-----���_�f�[�^�̃}�b�v-----*/
	char* vertMap = nullptr;
	hr = vertexBuffer->Map(0, nullptr, (void**)&vertMap);
	if (FAILED(hr)) {
		std::cout << "Failed to Map vertex\n";
		return hr;
	}
	std::copy(std::begin(PMDvertices), std::end(PMDvertices), vertMap);
	vertexBuffer->Unmap(0, nullptr);

	/*-----VertexBufferView�̐���-----*/
	vbView = {};
	vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vbView.SizeInBytes = PMDvertices.size();
	vbView.StrideInBytes = pmdvertex_size;

	/*-----�C���f�b�N�X���̊m�F-----*/
	unsigned int indicesNum;
	fread(&indicesNum, sizeof(indicesNum), 1, fp);
	std::cout << "���f���C���f�b�N�X�� : " << indicesNum << "\n";

	/*-----�C���f�b�N�X�f�[�^�̓ǂݍ���-----*/
	indices.resize(indicesNum);
	fread(indices.data(), indices.size() * sizeof(indices[0]), 1, fp);

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
	unsigned short* idxMap = nullptr;
	hr = indexBuffer->Map(0, nullptr, (void**)&idxMap);
	if (FAILED(hr)) {
		std::cout << "Failed to Map index\n";
		return hr;
	}
	std::copy(std::begin(indices), std::end(indices), idxMap);
	indexBuffer->Unmap(0, nullptr);

	/*-----IndexBufferView�̐����@-----*/
	ibView = {};
	ibView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = indices.size() * sizeof(indices[0]);

	/*-----�}�e���A���f�[�^�̓ǂݍ���-----*/
	//�}�e���A�����̊m�F
	unsigned int materialNum;
	fread(&materialNum, sizeof(materialNum), 1, fp);
	std::cout << "�}�e���A���� : " << materialNum << "\n";
	//�}�e���A���f�[�^�̓ǂݍ���
	std::vector<PMDMaterial> pmdMaterials(materialNum);
	fread(pmdMaterials.data(), pmdMaterials.size() * sizeof(PMDMaterial), 1, fp);
	//�V�F�[�_�]���p�f�[�^�ƁACPU�ŎQ�Ƃ���݂̂̃f�[�^�ɕ�����
	std::vector<Material> materials(pmdMaterials.size());
	for (int i = 0; i < materials.size(); i++) {
		materials[i].indicesNum = pmdMaterials[i].indicesNum;
		materials[i].material.diffuse = pmdMaterials[i].diffuse; 
		materials[i].material.alpha = pmdMaterials[i].alpha;
		materials[i].material.specular = pmdMaterials[i].specular; 
		materials[i].material.specularity = pmdMaterials[i].specularity; 
		materials[i].material.ambient = pmdMaterials[i].ambient;
	}

	/*-----MaterialBuffer�̐���-----*/


	/*-----�{�[�����̓ǂݍ���-----*/
	//�{�[�����̊m�F
	unsigned short boneNum = 0;
	fread(&boneNum, sizeof(boneNum), 1, fp);
	std::cout << "�{�[���� : " << boneNum << "\n";

	/*-----�{�[���f�[�^�̓ǂݍ���-----*/
	std::vector<PMDBone> pmdBones(boneNum);
	fread(pmdBones.data(), sizeof(PMDBone), boneNum, fp);
	/*for (int i = 0; i < boneNum; i++) {
		std::cout << "�{�[��[" << i << "] �{�[���� : " << pmdBones[i].boneName << "\n";
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

//�e�s���Ƃɏ���
/*
obj�t�@�C��
# : �R�����g
v : ���_���W
vt : �e�N�X�`�����W
vn : �@��
f : �ʏ��
���ǂݍ��ݕ��j
�E1�s���ǂݍ���
�E�ŏ��̕����ɉ����ď�������
*/
HRESULT Object::LoadOBJData(std::string ModelName, ID3D12Device* device) {
	//�I�u�W�F�N�g�̑��d���[�h��h��
	if (ObjectLoaded) {
		std::cout << "Object File is already loaded\n";
		return S_OK;
	}
	ObjectLoaded = true;

	std::cout << "\n----------wavefrontOBJ�ǂݍ��݊J�n----------\n";

	HRESULT hr;
	std::string strModelPath = "Model/" + ModelName + ".obj"; //�I�u�W�F�N�g�t�@�C���p�X�̐���
	std::ifstream file(strModelPath); //�I�u�W�F�N�g�t�@�C���̓ǂݍ���
	if (!file) {
		std::cout << "cannnot find model file\n";
		return FALSE;
	}

	//��s���ǂݍ��݁A[v],[vt],[vn],[f]���擾
	std::vector<XMFLOAT3> v; //���_���W
	std::vector<XMFLOAT2> vt; //�e�N�X�`�����W
	std::vector<XMFLOAT3> vn; //�@�����
	std::vector<OBJFaceInfo> f; //�ʏ��(��������string�̂܂܎擾)

	FILE* fp;
	fp = fopen(strModelPath.c_str(), "r");
	if (fp == NULL) {
		std::cout << "cannnot open model file\n";
	}

	char lineData[128]; //1�s�f�[�^
	//1�s��������Ŏ擾�A�������ϊ����f�[�^�Ƃ��ċL�^���Ă���
	float time = clock();
	std::cout << "�e�L�X�g�f�[�^�ǂݎ��J�n : " << time / 1000.0f << "�b\n";
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
			//�ʏ��̐���3�̏ꍇ��4�̏ꍇ
			if (data.size() == 4) {
				info.fi[3] = "Nothing";
				f.push_back(info);
			}
			else if (data.size() == 5) {
				info.fi[3] = data[4];
				//vertNum++;
				f.push_back(info);
			}
			//�ʏ��̐���6�ȏ�̎�
			else if (data.size() > 5) {
				//����
			}
		}
	}
	std::cout << "�e�L�X�g�f�[�^�ǂݎ��I�� : " << clock() / 1000.0f << "�b\n�e�L�X�g�f�[�^�ǂݎ�莞�� : " << (clock() - time) / 1000 << "�b\n";

	std::cout << "���_�� : " << v.size() << "\n";
	int fSize = sizeof(OBJFaceInfo) / sizeof(std::string); //faceInfo�̂���fi�̗v�f��
	
	time = clock();
	std::cout << "�f�[�^��͊J�n : " << time / 1000.0f << "�b\n";

	unsigned indexNum = 0; //�C���f�b�N�Xid �꒸�_�������邽�тɃC���N�������g����
	//�ʏ������ƂɁA�@���_���̊i�[�ƒ��_�C���f�b�N�X�̐���;
	for (int i = 0; i < f.size(); i++) {
		/*
		"/"�Ȃ��ꍇ : ���_�̂�
		"a/b"�̏ꍇ : ���_/�e�N�X�`�����W
		"a//b"�̏ꍇ : ���_//�@��
		"a/b/c"�̏ꍇ : ���_/�e�N�X�`�����W/�@��
		*/

		//�܂��ʏ��𕪊�����
		/*
		obj�t�@�C���́A�ʏ��(f)�ɒ��_�ƒ��_�ɑΉ�����UV���W����і@���̏�񂪂���
		�܂�f�̈�v�f���꒸�_�ƂȂ�
		*/

		for (int j = 0; j < fSize - 1; j++) { //f[i].fi[2]�܂ŁB[3]��"Nothing"������ȊO���ŏ�������
			//std::cout << f[i].fi[j] << " ";
			std::vector<std::string > faceData = split(f[i].fi[j], "/");
			OBJVertex tmpVert; //OBJvertices�ꎞ�ۊǗp
			//fi[3] �� nothing������ȊO���ŏ������ς��
			//nothing�ł͂Ȃ��ꍇ�A4���_�ł���̂ŁA�O�p�`�𐶐����邽�߂ɃC���f�b�N�X���g������K�v������
			
			switch (faceData.size()) {
			case 1: //���_�f�[�^�̂�
				tmpVert.pos = v[std::stoi(faceData[0]) - 1]; //obj�t�@�C���̒��_id�͂Ȃ���1�X�^�[�g�Ȃ̂�-1��Y�ꂸ��
				tmpVert.uv = XMFLOAT2(0.0f, 0.0f); //�f�t�H���g��0
				tmpVert.normal = XMFLOAT3(0.0f, 0.0f, 0.0f); //�f�t�H���g��0
				break;
			case 2: //���_/�e�N�X�`�����W
				tmpVert.pos = v[std::stoi(faceData[0]) - 1];
				tmpVert.uv = vt[std::stoi(faceData[1]) - 1];
				tmpVert.normal = XMFLOAT3(0.0f, 0.0f, 0.0f); //�f�t�H���g��0
				break;
			case 3: //���_//�@�� or ���_/�e�N�X�`�����W/�@��
				if (faceData[1].length() == 0) { //���_//�@��
					tmpVert.pos = v[std::stoi(faceData[0]) - 1];
					tmpVert.uv = XMFLOAT2(0.0f, 0.0f);
					tmpVert.normal = vn[std::stoi(faceData[2]) - 1];
				}
				else { //���_/�e�N�X�`�����W/�@��
					tmpVert.pos = v[std::stoi(faceData[0]) - 1];
					tmpVert.uv = vt[std::stoi(faceData[1]) - 1];
					tmpVert.normal = vn[std::stoi(faceData[2]) - 1];
				}
				break;
			}
			OBJvertices.push_back(tmpVert);
			indices.push_back(indexNum);
			indexNum++;
			//vector�̉��
			vectorRelease(faceData);
		}
		//�ʐ���3�̏ꍇ
		if (f[i].fi[3] == "Nothing") { //�ʐ���5�ȏ�̏ꍇ������̂ŁA�����4�ڂ�Nothing��}�����A����ȍ~�͖������Ă���
			//�������Ȃ�
		}
		//�ʐ���4�̏ꍇ
		else {
			OBJVertex tmpVert; //OBJvertices�ꎞ�ۊǗp
			std::vector<std::string > faceData = split(f[i].fi[3], "/");

			switch (faceData.size()) {
			case 1: //���_�f�[�^�̂�
				tmpVert.pos = v[std::stoi(faceData[0]) - 1]; //obj�t�@�C���̒��_id�͂Ȃ���1�X�^�[�g�Ȃ̂�-1��Y�ꂸ��
				tmpVert.uv = XMFLOAT2(0.0f, 0.0f); //�f�t�H���g��0
				tmpVert.normal = XMFLOAT3(0.0f, 0.0f, 0.0f); //�f�t�H���g��0
				break;
			case 2: //���_/�e�N�X�`�����W
				tmpVert.pos = v[std::stoi(faceData[0]) - 1];
				tmpVert.uv = vt[std::stoi(faceData[1]) - 1];
				tmpVert.normal = XMFLOAT3(0.0f, 0.0f, 0.0f); //�f�t�H���g��0
				break;
			case 3: //���_//�@�� or ���_/�e�N�X�`�����W/�@��
				if (faceData[1].length() == 0) { //���_//�@��
					tmpVert.pos = v[std::stoi(faceData[0]) - 1];
					tmpVert.uv = XMFLOAT2(0.0f, 0.0f);
					tmpVert.normal = vn[std::stoi(faceData[2]) - 1];
				}
				else { //���_/�e�N�X�`�����W/�@��
					tmpVert.pos = v[std::stoi(faceData[0]) - 1];
					tmpVert.uv = vt[std::stoi(faceData[1]) - 1];
					tmpVert.normal = vn[std::stoi(faceData[2]) - 1];
				}
				break;
			}
			OBJvertices.push_back(tmpVert);

			//�Ⴆ��0,1,2,3�̏ꍇ
			//.obj�͒��_���E���Ȃ̂�, 0,1,2 ��0,2,3�ɂȂ�
			indices.push_back(indexNum-3);
			indices.push_back(indexNum-1);
			indices.push_back(indexNum);
			indexNum++;
		}
	}
	std::cout << "�f�[�^��͏I�� : " << clock() / 1000.0f << "�b\n�f�[�^��͎��� : " << (clock() - time) / 1000 << "�b\n";
	
	vectorRelease(v);
	vectorRelease(vt);
	vectorRelease(vn);
	vectorRelease(f);

	/*-----VertexBuffer�̐���-----*/
	//HeapProperty�̐ݒ�
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	//ResourcDesc�̒�`
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
	std::copy(std::begin(OBJvertices), std::end(OBJvertices), vertMap);
	vertexBuffer->Unmap(0, nullptr);

	/*-----VertexBufferView�̐���-----*/
	vbView = {};
	vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vbView.SizeInBytes = OBJvertices.size() * sizeof(OBJVertex);
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

	/*-----IndexBufferView�̐����@-----*/
	ibView = {};
	ibView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R32_UINT;
	ibView.SizeInBytes = indices.size() * sizeof(indices[0]);

	std::cout << "----------wavefrontOBJ�ǂݍ��ݏI��----------\n";

	return S_OK;
}

//�w�肵��������splitter�ŕ�����str�𕪊�
std::vector<std::string> Object::split(std::string str, std::string splitter) {
	std::vector<std::string> str_s;
	str_s.reserve(8);
	int offset = 0; //������T���J�n�ʒu
	int splitPos = 0; //�����_�ʒu

	//�ŏ��̕����_�T��
	splitPos = str.find(splitter, offset);
	if (splitPos == -1) {
		str_s.push_back(str);
		return str_s; //���������_�����݂��Ȃ������炱�̎��_�ŏI��,����������̂܂ܕԂ�
	}
	while (splitPos != -1) {
		str_s.push_back(str.substr(offset, splitPos - offset));

		//�I�t�Z�b�g�ʒu�Ǝ��̕����_��T��
		offset = splitPos + 1; //���̒T���ɁA�����ς݂̕����_���܂܂Ȃ�
		splitPos = str.find(splitter, offset);
	}
	//�Ō�̈��
	str_s.push_back(str.substr(offset));

	return str_s;
}

template<typename T>
void Object::vectorRelease(std::vector<T>& vec) {
	if (vec.data() == nullptr) return; //nullptr�Ȃ�֐��I��
	//�ŋ߂�vector�̉��(size��0�ɂ��Ă���capacity��0��)�̎d���炵��
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