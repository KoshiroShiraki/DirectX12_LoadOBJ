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
		*(path + 1) = '\0';
	}
}

void PathController::AddLeafPath(const char* c1, char* path, const char* addPath, size_t pathLength) {
	size_t len = strlen(c1);
	size_t addLen = strlen(addPath);
	if ((len + addLen + 2) > pathLength) {
		std::cout << "Error : Path Length is too Long" << std::endl;
	}
	else {
		memcpy(path, c1, len);
		path += len;
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

int PathController::PathFinder(const char* defaultPath, char* path, const char* startDir, size_t pathLength) {
	char checkDir[MAX_PATH_LENGTH];
	HANDLE hFind;
	WIN32_FIND_DATA fd;

	int cpySize = 0;
	bool isSlash = false; //check there is at least one back slash in Path, if there is no back slash, path stores defaultPath
	while (*(defaultPath + cpySize) != '\0') {
		cpySize++;
		if (*(defaultPath + cpySize) == '\\') {
			isSlash = true;
			memcpy(checkDir, defaultPath, cpySize);
			checkDir[cpySize] = '\0';

			char checkPath[MAX_PATH_LENGTH];
			AddLeafPath(startDir, checkPath, checkDir);

			hFind = FindFirstFile(checkPath, &fd);
			if (hFind != INVALID_HANDLE_VALUE) { //Find checkPath
				AddLeafPath(startDir, path, defaultPath, MAX_PATH_LENGTH);
				return 1;
			}
			defaultPath += cpySize + 1;
			cpySize = 0;
		}
	}

	if (!isSlash) {
		memcpy(path, defaultPath, strlen(defaultPath) + 1);
		
		char checkPath[MAX_PATH_LENGTH];
		AddLeafPath(startDir, checkPath, checkDir);

		hFind = FindFirstFile(checkPath, &fd);
		if (hFind != INVALID_HANDLE_VALUE) { //Find checkPath
			AddLeafPath(startDir, path, defaultPath, MAX_PATH_LENGTH);
			return 1;
		}
	}
	else return -1;
}