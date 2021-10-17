#include"PathController.h"

PathController::PathController() {
	/*-----Create basePath-----*/
	GetCurrentDirectory(MAX_PATH_LENGTH, basePath);
	char *deleteDir = std::strstr(basePath, baseDirName);

	while (*deleteDir != '\\') {
		deleteDir++;
	}

	*deleteDir = '\0';
}

PathController::~PathController() {

}

void PathController::CreatePath(const char* c1, char* path, size_t pathLength) {
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
	if (len + 1 > pathLength) {
		std::cout << "Error : Path Length is too Long" << std::endl;
	}
	else {
		memcpy(path, c1, len);
		path += len;
		while (*path != '\\') {
			path--;
		}
		*path = '\0';
	}
}

void PathController::AddLeafPath(const char* c1, char* path, const char* addPath, size_t pathLength) {
	size_t len = strlen(c1);
	size_t addLen = strlen(addPath);
	//メモリオーバーしないかチェック
	if ((len + addLen + 2) > pathLength) {
		std::cout << "Error : Path Length is too Long" << std::endl;
	}
	else {
		memcpy(path, c1, len);
		path += len;
		*path = '\\';
		path++;
		memcpy(path, addPath, addLen + 1);
	}
}

void PathController::GetLeafDirectryName(const char* c1, char* name, size_t nameLength) {
	int cpySize = 0;
	
	c1 += strlen(c1);

	while (*c1 != '\\') {
		c1--;
		cpySize++;
	}
	
	if (cpySize > nameLength) {
		std::cout << "Error : DirectoryName is too Long" << std::endl;
	}
	else {
		memcpy(name, c1 + 1, cpySize);
	}
}

void PathController::GetAfterPathFromDirectoryName(const char* c1, char* path, const char* DirectoryName, size_t pathLength) {
	int cpySize = 0;
	c1 = strstr(c1, DirectoryName);
	if (c1 == NULL) {
		std::cout << "Error : There is no DirectryName in the path" << std::endl;
	}
	else {
		
		while (*c1 != '\\') {
			c1++;
		}

		for (cpySize = 0; *(c1 + cpySize) != '\0'; cpySize++);
		if (cpySize > pathLength) {
			std::cout << "Error : Path Length is too Long" << std::endl;
		}
		else {
			memcpy(path, c1 + 1, cpySize);
		}
	}
}