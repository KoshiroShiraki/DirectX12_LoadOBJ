#pragma once
#include<Windows.h>
#include<iostream>

#define MAX_PATH_LENGTH 1024

class PathController {
public:
	PathController();
	~PathController();
	PathController(const PathController &pc);

private:
	const char* baseDirName = "ObjectViewer"; //this is Project Directory Name. all Files/Directories releted this Application must be inluceded in this directory

public:
	char basePath[MAX_PATH_LENGTH]; //基準ディレクトリパス

public:
	void CreatePath(const char* c1, char* path, size_t pathLength = MAX_PATH_LENGTH); //c1 = string added to basePath, path = variable which store the CreatedPath, pathLength = path's length
	void RemoveLeafPath(const char* c1, char* path, size_t pathLength = MAX_PATH_LENGTH); //c1 = original string, path = variable which store the CreatedPath, pathLength = path's length
	void AddLeafPath(const char* c1, char* path, const char* addPath, size_t pathLength = MAX_PATH_LENGTH); //c1 = original string, path = variable which store the CreatedPath, addPath = string added to c1, pathLength = path's length
	void GetLeafDirectryName(const char* c1, char* name, size_t nameLength); //c1 = original string, name = variable which store the CreatedPath, nameLength = name's length
	void GetAfterPathFromDirectoryName(const char* c1, char* path, const char* DirectoryName, size_t pathLength = MAX_PATH_LENGTH); //c1 = original path, path = variable which store CreatedPath, DirectoryName = Path after this Directory will be got, pathLength = path's length
	int PathFinder(const char* defaultPath, char* path, const char* startDir, size_t pathLength = MAX_PATH_LENGTH);
};