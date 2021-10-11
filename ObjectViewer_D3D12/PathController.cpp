#include"PathController.h"

PathController::PathController() {
	//�x�[�X�f�B���N�g���p�X�̐���
	GetCurrentDirectory(MAX_PATH_LENGTH, basePath); //�J�����g�f�B���N�g���̎擾
	char *deleteDir = std::strstr(basePath, baseDirName); //�x�[�X�f�B���N�g�����̂���|�C���^���擾����

	//�x�[�X�f�B���N�g�������玟��\�܂ňړ�
	while (*deleteDir != '\\') {
		deleteDir++;
	}

	*deleteDir = '\0'; //null�I�[���������
}

PathController::~PathController() {

}

void PathController::CreatePath(const char* c1, char* path, size_t pathLength) {
	//�������I�[�o�[���Ȃ����`�F�b�N
	if ((strlen(basePath) + strlen(c1) + 1) > pathLength) {
		std::cout << "Error : Path Length is too Long" << std::endl;
	}
	else {
		memcpy(path, basePath, strlen(basePath) + 1);
		strcat_s(path, pathLength, c1);
	}
}

void PathController::RemoveLeafPath(const char* c1, char* path, size_t pathLength) {
	size_t len = strlen(c1);
	//�������I�[�o�[���Ȃ����`�F�b�N
	if (len + 1 > pathLength) {
		std::cout << "Error : Path Length is too Long" << std::endl;
	}
	else {
		memcpy(path, c1, len); //c1��const char*�Ȃ̂Ő�ɒ��g��path�ɃR�s�[
		path += len; //�|�C���^�ʒu��null�I�[�����܂ňړ�
		while (*path != '\\') { //�Ō�̋t�X���b�V����������
			path--;
		}
		//���������ʒu��null�I�[�����ɂ���
		*path = '\0';
	}
}

void PathController::AddLeafPath(const char* c1, char* path, const char* addPath, size_t pathLength) {
	size_t len = strlen(c1);
	size_t addLen = strlen(addPath);
	//�������I�[�o�[���Ȃ����`�F�b�N
	if ((len + addLen + 2) > pathLength) { //����͌��̃p�X��\��ǉ��������addPath���ǉ�����̂ŁAnull�I�[������ƍ��킹�� +2 ����K�v������
		std::cout << "Error : Path Length is too Long" << std::endl;
	}
	else {
		memcpy(path, c1, len); //path��c1��null�I�[�����ȊO���R�s�[
		path += len; //c1�̕������R�s�[����Ă���ʒu�̉��܂ňړ�
		*path = '\\'; //�t�@�C���p�X��؂��}��
		path++; //��i�߂�
		memcpy(path, addPath, addLen + 1); //null�I�[�ʒu�܂ŃR�s�[
	}
}

void PathController::GetLeafDirectryName(const char* c1, char* name, size_t nameLength) {
	int cpySize = 0; //�R�s�[���閼�O�̑傫��
	
	//c1��null�I�[�����ʒu�܂ňړ�
	c1 += strlen(c1);

	while (*c1 != '\\') { //�o�b�N�X���b�V����������܂�
		c1--;
		cpySize++;
	}
	//�������I�[�o�[���Ȃ����`�F�b�N
	if (cpySize > nameLength) {
		std::cout << "Error : DirectoryName is too Long" << std::endl;
	}
	else {
		memcpy(name, c1 + 1, cpySize); //c1�͂���'\\'�ɂ���̂ŁA�ЂƂO�ɐi�߂Ă���ccpySize�� = null�I�[�������݂̕��������R�s�[����
	}
}

void PathController::GetAfterPathFromDirectoryName(const char* c1, char* path, const char* DirectoryName, size_t pathLength) {
	int cpySize = 0; //�R�s�[����p�X�̑傫��
	//DirectoryName�ʒu�Ɉړ�����
	c1 = strstr(c1, DirectoryName);
	if (c1 == NULL) {
		std::cout << "Error : There is no DirectryName in the path" << std::endl;
	}
	else {
		//����\�܂ňړ�����
		while (*c1 != '\\') {
			c1++;
		}

		for (cpySize = 0; *(c1 + cpySize) != '\0'; cpySize++); //�k���I�[�����܂ł̕��������擾����
		if (cpySize > pathLength) {
			std::cout << "Error : Path Length is too Long" << std::endl;
		}
		else {
			memcpy(path, c1 + 1, cpySize);
		}
	}
}