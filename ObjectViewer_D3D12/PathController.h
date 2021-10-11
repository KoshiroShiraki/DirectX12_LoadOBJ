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
	const char* baseDirName = "ObjectViewer"; //基準ディレクトリ名　アプリ内のすべてのファイルはこのディレクトリからの相対パスで指定する

public:
	char basePath[MAX_PATH_LENGTH]; //基準ディレクトリパス

public:
	void CreatePath(const char* c1, char* path, size_t pathLength = MAX_PATH_LENGTH); //受け取ったc1をbasePathに結合し、pathにコピーする。
	//c1 = 結合する文字列, path = 結合された文字列を格納するポインタ, pathLength = pathの確保されているメモリサイズ
	void RemoveLeafPath(const char* c1, char* path, size_t pathLength = MAX_PATH_LENGTH); //一番左のディレクトリを削除したパスをpathに渡す
	//path = 全体パス, pos = 削除された文字列を格納するポインタ
	void AddLeafPath(const char* c1, char* path, const char* addPath, size_t pathLength = MAX_PATH_LENGTH); //c1のディレクトリにpathを追加する
	//c1 = 全体パス, path = 追加された文字列を格納するポインタ, addPath = 追加する文字列
	void GetLeafDirectryName(const char* c1, char* name, size_t nameLength); //パスの一番先端のディレクトリ名を取得しnameに格納する
	//c1 = 全体パス, name = ディレクトリ名格納ベクトル
	void GetAfterPathFromDirectoryName(const char* c1, char* path, const char* DirectoryName, size_t pathLength = MAX_PATH_LENGTH);
};