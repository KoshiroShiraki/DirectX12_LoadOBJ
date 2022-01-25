#pragma once
#include"DX12ObjectFormatOBJ.h"

DX12ObjectFormatOBJ::DX12ObjectFormatOBJ() {
	m_transform.position = XMFLOAT3(0, 0, 0);
	m_transform.rotation = XMFLOAT3(0, 0, 0);
	m_transform.size = XMFLOAT3(1, 1, 1);

	m_wMatrix = XMMatrixIdentity();
}

DX12ObjectFormatOBJ::DX12ObjectFormatOBJ(DX12ObjectFormatOBJ* origin, ID3D12Device* device) {
	m_name = origin->m_name + "(copy)";

	m_transform.position = origin->m_transform.position;
	m_transform.rotation = origin->m_transform.rotation;
	m_transform.size = origin->m_transform.size;

	//�����s��̓I���W�i�����f���̃p�����[�^�𔽉f
	XMMATRIX posMat = XMMatrixTranslation(m_transform.position.x, m_transform.position.y, m_transform.position.z);
	XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(m_transform.rotation.x * XM_PI / 180, m_transform.rotation.y * XM_PI / 180, m_transform.rotation.z * XM_PI / 180);
	XMMATRIX sizMat = XMMatrixScaling(m_transform.size.x, m_transform.size.y, m_transform.size.z);

	m_wMatrix = sizMat * rotMat * posMat; //Create New Matrix to hand mapMatrix

	m_obj.resize(origin->m_obj.size()); //�K�w���f���̐���
	for (int i = 0; i < m_obj.size(); i++) {
		m_obj[i] = new DX12Object3D();
		//���_�ƃC���f�b�N�X�̃R�s�[���
		m_obj[i]->m_vertices.resize(origin->m_obj[i]->m_vertices.size());
		std::copy(origin->m_obj[i]->m_vertices.begin(), origin->m_obj[i]->m_vertices.end(), m_obj[i]->m_vertices.begin());
		m_obj[i]->m_indices.resize(origin->m_obj[i]->m_indices.size());
		std::copy(origin->m_obj[i]->m_indices.begin(), origin->m_obj[i]->m_indices.end(), m_obj[i]->m_indices.begin());
		//�}�e���A���̃R�s�[���
		memcpy_s(&m_obj[i]->m_material, sizeof(DX12Material), &origin->m_obj[i]->m_material, sizeof(DX12Material));

		m_obj[i]->Create(device); //����
	}
}

DX12ObjectFormatOBJ::~DX12ObjectFormatOBJ() {
	for (int i = 0; i < m_obj.size(); i++) {
		if (m_obj[i] != nullptr) {
			delete(m_obj[i]);
			m_obj[i] = nullptr;
		}
	}
	m_obj.clear();
	m_obj.shrink_to_fit();
}

HRESULT DX12ObjectFormatOBJ::LoadVertexFromOBJFile(std::string filePath, ID3D12Device* device) {
	HRESULT hr;

	std::cout << "\n-------------------------\n";
	std::cout << "���f���ǂݍ��݊J�n" << std::endl;
	float loadTime = clock();

	//���b�V���t�@�C���̐�΃p�X�̎擾
	PathController pc;
	char meshPath[MAX_PATH_LENGTH];
	pc.CreatePath(filePath.c_str(), meshPath);
	char fmdPath[MAX_PATH_LENGTH];

	//fmd(flatbuffers model data)�t�@�C���̐�΃p�X
	pc.ReplaceExtension(meshPath, fmdPath, "fmd");

	//���f����
	char tmp[256];
	pc.GetLeafDirectryName(meshPath, tmp, 256);
	char name[256];
	pc.ReplaceExtension(tmp, name, nullptr, 256);

	//�o�C�i���f�[�^���J��
	std::ifstream ifs(fmdPath, std::ios::binary);
	//�J���Ȃ� = �܂����݂��Ă��Ȃ��̂Œʏ�ǂݍ���
	if (!ifs) {

		//���b�V���t�@�C�����J��
		FILE* modelFp;
		if (fopen_s(&modelFp, meshPath, "r") != 0) {
			return ErrorMessage("Failed to Open MeshFile : " + filePath);
		}

		//���_�f�[�^�ꎞ�ۊǗp�z��
		std::vector<XMFLOAT3> v;
		std::vector<XMFLOAT2> vt;
		std::vector<XMFLOAT3> vn;
		std::vector<std::vector<int*>> fd;
		std::vector<unsigned> vertexCntPerObj; //�I�u�W�F�N�g���Ƃ̒��_��
		std::vector<std::string> splitData;
		std::vector<int> obj_splitPos;

		v.reserve(524288);
		vt.reserve(524288);
		vn.reserve(524288);

		v.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
		vt.push_back(XMFLOAT2(0.0f, 0.0f));
		vn.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));

		char lineData[MAX_COUNT_LINEDATA];

		int objCnt = 0;

		while (fgets(lineData, MAX_COUNT_LINEDATA, modelFp) != NULL) {
			splitBlank(lineData, splitData);
			if (splitData[0] == "v") {
				try {
					v.push_back(XMFLOAT3(std::stof(splitData[1]), std::stof(splitData[2]), std::stof(splitData[3])));
				}
				catch (std::exception& e) {
					v.push_back(XMFLOAT3(0, 0, 0));
				}
			}
			else if (splitData[0] == "vt") {
				try {
					vt.push_back(XMFLOAT2(std::stof(splitData[1]), std::stof(splitData[2])));
				}
				catch (std::exception& e) {
					vt.push_back(XMFLOAT2(0, 0));
				}
			}
			else if (splitData[0] == "vn") {
				try {
					vn.push_back(XMFLOAT3(std::stof(splitData[1]), std::stof(splitData[2]), std::stof(splitData[3])));
				}
				catch (std::exception& e) {
					vn.push_back(XMFLOAT3(0, 0, 0));
				}
			}
			else if (splitData[0] == "f") {
				std::vector<int*> tmpData(splitData.size() - 1);
				for (int i = 0; i < tmpData.size(); i++) {
					tmpData[i] = (int*)malloc(sizeof(int) * 3);
					splitSlash(splitData[i + 1], tmpData[i]);
				}
				fd.push_back(tmpData);
			}
			else if (splitData[0] == "usemtl") {
				if (objCnt > 0) {
					obj_splitPos.push_back(fd.size() - 1);
				}
				objCnt++;
			}

			splitData.clear();
		}
		//�t�@�C�������
		fclose(modelFp);

		obj_splitPos.push_back(fd.size() - 1);

		//�f�[�^�̐���
		unsigned indexNum = 0;
		std::vector<DX12Vertex> vertices;
		std::vector<unsigned> indices;
		m_obj.resize(objCnt);
		int offset = 0;
		int k = 0;

		for (int i = 0; i < fd.size(); i++) {
			//���_�f�[�^
			for (int j = 0; j < fd[i].size(); j++) {
				DX12Vertex tmpVert;
				tmpVert.position = v[fd[i][j][0]];
				tmpVert.uv = vt[fd[i][j][1]];
				tmpVert.normal = vn[fd[i][j][2]];
				vertices.push_back(tmpVert);
			}
			//�C���f�b�N�X�f�[�^
			int indexOffset = indexNum;
			for (int j = 0; j < (fd[i].size() - 2); j++) {
				indices.push_back(indexOffset);
				indices.push_back(indexOffset + 1 + j);
				indices.push_back(indexOffset + 2 + j);
			}
			indexNum += fd[i].size();

			//3D�I�u�W�F�N�g����
			if (i == obj_splitPos[k]) {
				m_obj[k] = new DX12Object3D();
				m_obj[k]->m_name = "Material" + std::to_string(k);

				m_obj[k]->m_vertices.resize(vertices.size());
				std::copy(vertices.begin(), vertices.end(), m_obj[k]->m_vertices.begin());
				vertices.clear();

				m_obj[k]->m_indices.resize(indices.size());
				std::copy(indices.begin(), indices.end(), m_obj[k]->m_indices.begin());
				indices.clear();

				m_obj[k]->Create(device);
				if (k + 1 < obj_splitPos.size()) k++;
				indexNum = 0;
			}
		}

		m_name = name;

		//Flatbuffers���g�p���A�V���A���C�Y���ĕۑ�
		SaveFMDFile();
	}

	else { //fmd�t�@�C�������݂���ꍇ
		ifs.seekg(0, std::ios::end);
		size_t size = ifs.tellg();
		ifs.seekg(0, std::ios::beg);
		char* bbuf = new char[size];
		ifs.read(bbuf, size);
		ifs.close();
		auto data = DX12ModelData::GetModelParent(bbuf);

		//�f�[�^��^���Ă���
		m_obj.resize(data->child()->size());
		for (int i = 0; i < m_obj.size(); i++) {
			m_obj[i] = new DX12Object3D();
			m_obj[i]->m_name = "Material" + std::to_string(i);
			m_obj[i]->m_vertices.resize(data->child()->Get(i)->vertices()->size());
			//���_�̃R�s�[
			for (int j = 0; j < m_obj[i]->m_vertices.size(); j++) {
				//�ʒu
				m_obj[i]->m_vertices[j].position.x = data->child()->Get(i)->vertices()->Get(j)->pos()->x();
				m_obj[i]->m_vertices[j].position.y = data->child()->Get(i)->vertices()->Get(j)->pos()->y();
				m_obj[i]->m_vertices[j].position.z = data->child()->Get(i)->vertices()->Get(j)->pos()->z();
				//�@��
				m_obj[i]->m_vertices[j].normal.x = data->child()->Get(i)->vertices()->Get(j)->norm()->x();
				m_obj[i]->m_vertices[j].normal.y = data->child()->Get(i)->vertices()->Get(j)->norm()->y();
				m_obj[i]->m_vertices[j].normal.z = data->child()->Get(i)->vertices()->Get(j)->norm()->z();
				//UV
				m_obj[i]->m_vertices[j].uv.x = data->child()->Get(i)->vertices()->Get(j)->uv()->u();
				m_obj[i]->m_vertices[j].uv.y = data->child()->Get(i)->vertices()->Get(j)->uv()->v();
			}
			//�C���f�b�N�X�̃R�s�[
			m_obj[i]->m_indices.resize(data->child()->Get(i)->indices()->size());
			for (int j = 0; j < m_obj[i]->m_indices.size(); j++) {
				m_obj[i]->m_indices[j] = data->child()->Get(i)->indices()->Get(j);
			}
			m_obj[i]->Create(device);
		}
		m_name = name;
	}

	std::cout << "���f���ǂݍ��ݏI��(" << (clock() - loadTime) / 1000 << "�b)" << std::endl;

	return S_OK;
}

HRESULT DX12ObjectFormatOBJ::SaveFMDFile() {
	//�t���b�g�o�b�t�@����
	std::vector<flatbuffers::Offset<DX12ModelData::Vertex>> fbVertices_vector;
	//�r���_�[�̗p��
	flatbuffers::FlatBufferBuilder builder;
	//�q���f���z��
	std::vector<flatbuffers::Offset<DX12ModelData::ModelChild>> fbModelChild_vector;
	fbModelChild_vector.resize(m_obj.size());
	//�e���f��
	flatbuffers::Offset<DX12ModelData::ModelParent> fbModelParent;

	//�q���f���̐���
	for (int i = 0; i < m_obj.size(); i++) {
		fbVertices_vector.resize(m_obj[i]->m_vertices.size());
		for (int j = 0; j < fbVertices_vector.size(); j++) {
			DX12ModelData::Position pos = DX12ModelData::Position(m_obj[i]->m_vertices[j].position.x, m_obj[i]->m_vertices[j].position.y, m_obj[i]->m_vertices[j].position.z);
			DX12ModelData::Normal norm = DX12ModelData::Normal(m_obj[i]->m_vertices[j].normal.x, m_obj[i]->m_vertices[j].normal.y, m_obj[i]->m_vertices[j].normal.z);
			DX12ModelData::UV uv = DX12ModelData::UV(m_obj[i]->m_vertices[j].uv.x, m_obj[i]->m_vertices[j].uv.y);

			flatbuffers::Offset<DX12ModelData::Vertex> tmp;

			fbVertices_vector[j] = DX12ModelData::CreateVertex(builder, &pos, &norm, &uv);
		}

		//���_�f�[�^�̕ۑ�
		flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<DX12ModelData::Vertex>>> fbVertices = builder.CreateVector(fbVertices_vector);
		//�}�e���A���f�[�^�̕ۑ�
		DX12ModelData::Color ambColor = DX12ModelData::Color(m_obj[i]->m_material.ambient.x, m_obj[i]->m_material.ambient.y, m_obj[i]->m_material.ambient.z);
		DX12ModelData::Color difColor = DX12ModelData::Color(m_obj[i]->m_material.diffuse.x, m_obj[i]->m_material.diffuse.y, m_obj[i]->m_material.diffuse.z);
		DX12ModelData::Color speColor = DX12ModelData::Color(m_obj[i]->m_material.specular.x, m_obj[i]->m_material.specular.y, m_obj[i]->m_material.specular.z);
		float N = m_obj[i]->m_material.N;
		
		flatbuffers::Offset<DX12ModelData::Material> fbMaterial = DX12ModelData::CreateMaterial(builder, &ambColor, &difColor, &speColor, N);
		
		flatbuffers::Offset<flatbuffers::Vector<uint32_t>> fbIndices = builder.CreateVector(m_obj[i]->m_indices.data(), m_obj[i]->m_indices.size());
		fbModelChild_vector[i] = DX12ModelData::CreateModelChild(builder, fbVertices, fbMaterial, fbIndices);
	}

	//�o���オ�����o�C�i���f�[�^���o��(�g���q��.fmd = flatbuffers model data)
	flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<DX12ModelData::ModelChild>>> fbModelChild = builder.CreateVector(fbModelChild_vector);
	flatbuffers::Offset<flatbuffers::String> modelName = builder.CreateString(m_name);
	flatbuffers::Offset<DX12ModelData::ModelParent> ModelData = DX12ModelData::CreateModelParent(builder, modelName, fbModelChild);
	builder.Finish(ModelData);

	//�V���A���C�Y���ꂽ�o�b�t�@�f�[�^�̃|�C���^�擾
	uint8_t* buf = builder.GetBufferPointer();

	//�ۑ����邽�߂̃t�@�C���p�X�𐶐�
	char fmdPath[MAX_PATH_LENGTH];
	PathController pc;
	pc.CreatePath(("\\ObjectViewer_D3D12\\Model\\OBJ\\" + m_name + ".fmd").c_str(), fmdPath);

	//�ۑ�
	std::ofstream writeFile;
	writeFile.open(fmdPath, std::ios::out | std::ios::binary);
	writeFile.write((char*)buf, builder.GetSize());
	writeFile.close();

	return S_OK;
}

void DX12ObjectFormatOBJ::splitBlank(std::string str, std::vector<std::string>& data) {
	//�f�[�^�̏�����
	data.clear();

	//���������͋�
	std::string splitter = " ";
	std::string tmp;

	int offset = 0;
	int splitPos = 0;

	splitPos = str.find(splitter, offset);

	while (splitPos != -1) {
		tmp = str.substr(offset, splitPos - offset);

		//�󔒂��A�����Ă���ꍇ�̓v�b�V�����Ȃ�
		if (tmp.size() != 0) data.push_back(tmp); 

		offset = splitPos + 1;

		splitPos = str.find(splitter, offset);
	}

	//�Ō�̈��
	if (str.substr(offset).size() != 0 && str.substr(offset) != "\n")  data.push_back(str.substr(offset, (str.size() - offset - 1)));

	//������������������Ȃ�������A�k���I�[��������
	if (data.size() == 0) data.push_back("\0");
}

void DX12ObjectFormatOBJ::splitSlash(std::string str, int* data) {
	//���������̓X���b�V��
	std::string splitter = "/";
	std::string tmp;

	//�T���J�n�ʒu
	int offset = 0;
	//���������ʒu
	int splitPos = 0;

	splitPos = str.find(splitter, offset);

	int i = 0;
	while (splitPos != std::string::npos) {
		tmp = str.substr(offset, splitPos - offset);
		if (tmp.size() == 0) data[i] = 0;
		else data[i] = std::stoi(tmp);

		offset = splitPos + 1;
		splitPos = str.find(splitter, offset);

		i++;
		//�X���b�V����3�ȏ�͂��蓾�Ȃ��̂ŃG���[
		if (i > 2) {
			ErrorMessage("Too Many Slash");
			return;
		}
	}


	if (i == 1) {
		data[1] = std::stoi((str.substr(offset)));
		data[2] = 0;
	}
	else if (i == 2) {
		if (str.substr(offset).size() == 0) data[2] = 0;
		else data[2] = std::stoi((str.substr(offset)));
	}
}

void DX12ObjectFormatOBJ::UpdateTransform(float* data) {
	m_transform.position = XMFLOAT3(data[0], data[1], data[2]);
	m_transform.rotation = XMFLOAT3(data[3], data[4], data[5]);
	m_transform.size = XMFLOAT3(data[6], data[7], data[8]);

	XMMATRIX posMat = XMMatrixTranslation(m_transform.position.x, m_transform.position.y, m_transform.position.z);
	XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(m_transform.rotation.x * XM_PI / 180, m_transform.rotation.y * XM_PI / 180, m_transform.rotation.z * XM_PI / 180);
	XMMATRIX sizMat = XMMatrixScaling(m_transform.size.x, m_transform.size.y, m_transform.size.z);

	m_wMatrix = sizMat * rotMat * posMat; //Create New Matrix to hand mapMatrix
}