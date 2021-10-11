#pragma once
#include<Windows.h>
#include<iostream>

#define MAX_PATH_LENGTH 256

class PathController {
public:
	PathController();
	~PathController();
	PathController(const PathController &pc);

private:
	const char* baseDirName = "ObjectViewer"; //��f�B���N�g�����@�A�v�����̂��ׂẴt�@�C���͂��̃f�B���N�g������̑��΃p�X�Ŏw�肷��

public:
	char basePath[MAX_PATH_LENGTH]; //��f�B���N�g���p�X

public:
	void CreatePath(const char* c1, char* path, size_t pathLength = MAX_PATH_LENGTH); //�󂯎����c1��basePath�Ɍ������Apath�ɃR�s�[����B
	//c1 = �������镶����, path = �������ꂽ��������i�[����|�C���^, pathLength = path�̊m�ۂ���Ă��郁�����T�C�Y
	void RemoveLeafPath(const char* c1, char* path, size_t pathLength = MAX_PATH_LENGTH); //��ԍ��̃f�B���N�g�����폜�����p�X��path�ɓn��
	//path = �S�̃p�X, pos = �폜���ꂽ��������i�[����|�C���^
	void AddLeafPath(const char* c1, char* path, const char* addPath, size_t pathLength = MAX_PATH_LENGTH); //c1�̃f�B���N�g����path��ǉ�����
	//c1 = �S�̃p�X, path = �ǉ����ꂽ��������i�[����|�C���^, addPath = �ǉ����镶����
	void GetLeafDirectryName(const char* c1, char* name, size_t nameLength); //�p�X�̈�Ԑ�[�̃f�B���N�g�������擾��name�Ɋi�[����
	//c1 = �S�̃p�X, name = �f�B���N�g�����i�[�x�N�g��
	void GetAfterPathFromDirectoryName(const char* c1, char* path, const char* DirectoryName, size_t pathLength = MAX_PATH_LENGTH);
};