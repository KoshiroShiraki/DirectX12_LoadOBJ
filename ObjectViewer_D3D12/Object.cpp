#include"Object.h"
#pragma warning(disable: 4996)

Object::Object() {

}

Object::Object(std::string name) {
	this->objName = name;
}

Object::~Object() {
	Release();
}

HRESULT Object::LoadOBJData(std::string ModelName, ID3D12Device* device) {
	//�I�u�W�F�N�g�̑��d���[�h��h��
	if (ObjectLoaded) {
		std::cout << "Object File is already loaded\n";
		return S_OK;
	}
	ObjectLoaded = true;

	std::cout << "\n----------wavefrontOBJ�ǂݍ��݊J�n----------\n";

	float loadTime = clock(); //���[�h�J�n���Ԃ��擾

	HRESULT hr;

	std::string strModelPath = ModelName; //�I�u�W�F�N�g�t�@�C���p�X�̐���
	
	//��s���ǂݍ��݁A[v],[vt],[vn],[f]���擾
	std::vector<XMFLOAT3> v; //���_���W
	std::vector<XMFLOAT2> vt; //�e�N�X�`�����W
	std::vector<XMFLOAT3> vn; //�@�����
	std::vector<OBJFaceInfo> f; //�ʏ��(��������string�̂܂܎擾)
	std::vector<OBJFaceData> fd; //�ʃf�[�^

	//capacity�����炩���ߊm�ۂ��Ă����A�������̍Ċm�ۂ�h��
	//���f���f�[�^�����Ȃ��ꍇ�ɂ͖��ʂȊm�ۂ����A���[�h���I�����摬�₩�ɊJ������̂Ō���͖�莋���Ă��Ȃ�
	v.reserve(524288);
	vt.reserve(524288);
	vn.reserve(524288);
	f.reserve(524288);
	fd.reserve(524288);

	//�t�@�C�����[�h
	FILE* fp;
	fp = fopen(strModelPath.c_str(), "r");
	if (fp == NULL) {
		std::cout << "cannnot open model file\n";
	}
	else std::cout << "���f���t�@�C���p�X : " << strModelPath << "\n";


	char lineData[1024]; //1�s�f�[�^
	
	//1�s��������Ŏ擾�A�������ϊ����f�[�^�Ƃ��ċL�^���Ă���
	float time = clock();
	std::cout << "�e�L�X�g�f�[�^�ǂݎ��J�n : " << time / 1000.0f << "�b / ";

	//C++�ł̕������std::string����������邪�A�e�L�X�g�f�[�^�s�����傫���ꍇ��std::getline���Ɩ����ł��Ȃ����x���Ŏ��Ԃ�������̂ŁAC��fgets���g���Ă���
	while (fgets(lineData, 1024, fp) != NULL) { 
		std::vector<std::string> data = split(lineData, " ");
		if (data[0] == "v") { //���_
			v.push_back(XMFLOAT3(std::stof(data[1]), std::stof(data[2]), std::stof(data[3])));
		}
		else if (data[0] == "vt") { //UV
			vt.push_back(XMFLOAT2(std::stof(data[1]), std::stof(data[2])));
		}
		else if (data[0] == "vn") { //�@��
			vn.push_back(XMFLOAT3(std::stof(data[1]), std::stof(data[2]), std::stof(data[3])));
		}
		else if (data[0] == "f") { //��
			OBJFaceInfo tmpInfo; //�ꎞ�i�[�p
			tmpInfo.fi.reserve(data.size());
			for (int i = 0; i < data.size() - 1; i++) {
				tmpInfo.fi.push_back(data[i + 1]); //�f�[�^�͍ŏ���ID���(v�Ƃ�vn�Ƃ�vt�Ƃ�f)���L�^���Ă���̂ŁA�f�[�^�擾�ɂ�1�i�߂�K�v������
			}
			tmpInfo.faceNum = data.size() - 1;
			f.push_back(tmpInfo);
		}
	}
	std::cout << "�e�L�X�g�f�[�^�ǂݎ��I�� : " << clock() / 1000.0f << "�b\n�e�L�X�g�f�[�^�ǂݎ�莞�� : " << (clock() - time) / 1000 << "�b\n";

	std::cout << "���_ : " << v.size() << "�� / UV���W : " << vt.size() << "�� / �@�� : " << vn.size() << "�� / �� : " << f.size() << "��\n";
	
	time = clock();
	std::cout << "�f�[�^��͊J�n : " << time / 1000.0f << "�b / ";

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

		for (int j = 0; j < f[i].faceNum; j++) {
			//std::cout << f[i].fi[j] << " ";
			std::vector<std::string > faceData = split(f[i].fi[j], "/");
			OBJVertex tmpVert; //OBJvertices�ꎞ�ۊǗp
			
			/*for (int k = 0; k < faceData.size(); k++) {
				std::cout << faceData[k] << "\n";
			}*/
			//�ʏ��Ɋ܂܂����̐��ɉ����ĕ���
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
			//���_�f�[�^�փv�b�V��
			OBJvertices.push_back(tmpVert);
			//vector�̉��
			vectorRelease(faceData);
		}

		//f[i].faceNum�̌������ƂɃC���f�b�N�X�l�𐶐�
		//4���_�ɑΉ�����A���S���Y��(�œK�ł͂Ȃ�)
		int indexOffset = indexNum;
		for (int j = 0; j < (f[i].faceNum - 2); j++) {
			indices.push_back(indexOffset);
			indices.push_back(indexOffset + 1 + j);
			indices.push_back(indexOffset + 2 + j);
		}

		indexNum += f[i].faceNum;
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

	std::cout << "���f�����[�h���� : " << (clock() - loadTime) / 1000 << "�b\n";

	std::cout << "----------wavefrontOBJ�ǂݍ��ݏI��----------\n";

	return S_OK;
}

//�w�肵��������splitter�ŕ�����str�𕪊�
std::vector<std::string> Object::split(std::string str, std::string splitter) {
	std::vector<std::string> str_s;
	str_s.reserve(128);
	int offset = 0; //������T���J�n�ʒu
	int splitPos = 0; //�����_�ʒu

	//�ŏ��̕����_�T��
	splitPos = str.find(splitter, offset);
	if (splitPos == -1) {
		str_s.push_back(str);
		return str_s; //���������_�����݂��Ȃ������炱�̎��_�ŏI��,����������̂܂ܕԂ�
	}
	std::string tmp;
	while (splitPos != -1) {
		tmp = str.substr(offset, splitPos - offset);
		if(tmp.size() != 0) str_s.push_back(tmp);

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
	vectorRelease(OBJvertices);
	vectorRelease(indices);

	if (vertexBuffer) vertexBuffer->Release();
	if (indexBuffer) indexBuffer->Release();
	if (materialBuffer) materialBuffer->Release();
}