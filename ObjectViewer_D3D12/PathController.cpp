#include"PathController.h"

PathController::PathController() {
	//ベースディレクトリパスの生成
	GetCurrentDirectory(MAX_PATH_LENGTH, basePath); //カレントディレクトリの取得
	char *deleteDir = std::strstr(basePath, baseDirName); //ベースディレクトリ名のあるポインタを取得する

	//ベースディレクトリ名から次の\まで移動
	while (*deleteDir != '\\') {
		deleteDir++;
	}

	*deleteDir = '\0'; //null終端文字列を代入
}

PathController::~PathController() {

}

void PathController::CreatePath(const char* c1, char* path, size_t pathLength) {
	//メモリオーバーしないかチェック
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
	//メモリオーバーしないかチェック
	if (len + 1 > pathLength) {
		std::cout << "Error : Path Length is too Long" << std::endl;
	}
	else {
		memcpy(path, c1, len); //c1はconst char*なので先に中身をpathにコピー
		path += len; //ポインタ位置をnull終端文字まで移動
		while (*path != '\\') { //最後の逆スラッシュを見つける
			path--;
		}
		//見つかった位置をnull終端文字にする
		*path = '\0';
	}
}

void PathController::AddLeafPath(const char* c1, char* path, const char* addPath, size_t pathLength) {
	size_t len = strlen(c1);
	size_t addLen = strlen(addPath);
	//メモリオーバーしないかチェック
	if ((len + addLen + 2) > pathLength) { //今回は元のパスに\を追加した後にaddPathも追加するので、null終端文字列と合わせて +2 する必要がある
		std::cout << "Error : Path Length is too Long" << std::endl;
	}
	else {
		memcpy(path, c1, len); //pathにc1のnull終端文字以外をコピー
		path += len; //c1の文字がコピーされている位置の横まで移動
		*path = '\\'; //ファイルパス区切りを挿入
		path++; //一つ進める
		memcpy(path, addPath, addLen + 1); //null終端位置までコピー
	}
}

void PathController::GetLeafDirectryName(const char* c1, char* name, size_t nameLength) {
	int cpySize = 0; //コピーする名前の大きさ
	
	//c1をnull終端文字位置まで移動
	c1 += strlen(c1);

	while (*c1 != '\\') { //バックスラッシュが見つかるまで
		c1--;
		cpySize++;
	}
	//メモリオーバーしないかチェック
	if (cpySize > nameLength) {
		std::cout << "Error : DirectoryName is too Long" << std::endl;
	}
	else {
		memcpy(name, c1 + 1, cpySize); //c1はいま'\\'にいるので、ひとつ前に進めてからccpySize文 = null終端文字込みの文字数分コピーする
	}
}

void PathController::GetAfterPathFromDirectoryName(const char* c1, char* path, const char* DirectoryName, size_t pathLength) {
	int cpySize = 0; //コピーするパスの大きさ
	//DirectoryName位置に移動する
	c1 = strstr(c1, DirectoryName);
	if (c1 == NULL) {
		std::cout << "Error : There is no DirectryName in the path" << std::endl;
	}
	else {
		//次の\まで移動する
		while (*c1 != '\\') {
			c1++;
		}

		for (cpySize = 0; *(c1 + cpySize) != '\0'; cpySize++); //ヌル終端文字までの文字数を取得する
		if (cpySize > pathLength) {
			std::cout << "Error : Path Length is too Long" << std::endl;
		}
		else {
			memcpy(path, c1 + 1, cpySize);
		}
	}
}