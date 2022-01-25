#pragma once
#include<Windows.h>
#include<tchar.h>
#include<string>
#include<iostream>
#include<ShObjIdl_core.h>
#include"DirectXController.h"
#include"ConstValue.h"
#include"PathController.h"
#include"Input.h"
#include"MainWindowController.h"
#include"ListWindowController.h"
#include"EditWindowController.h"
#include"RenameWindowController.h"

class Application {
public:
	Application();
	~Application();

public:
	DirectXController DxCon; //描画用(DirectX12)
	Input input; //入力用
	Camera camera; //カメラ
	Light light; //ライト

	MainWindowController* m_mwc = nullptr; //メインウィンドウ(描画結果を表示)
	ListWindowController* m_lwc = nullptr; //リストウィンドウ(3Dモデルを管理)
	EditWindowController* m_ewc = nullptr; //エディットウィンドウ(描画パラメータの管理)
	RenameWindowController* m_rwc = nullptr; //リネームウィンドウ(モデルの名前を変更する)

	HINSTANCE hInst; //アプリケーションインスタンスハンドル

	std::vector<std::string> DefaultObjFilePaths; //読み込み可能3Dモデルファイル

	int objIndex = -1;

public:
	void UpdateComboBox(); //コンボボックスの中身を、DefaultObjFilePathsに更新する
	void UpdateListBox(); //モデルをロードし、リストぼっくしに新規に追加したり削除したりする
	void UpdateEditBox(); //選択されているモデルの持つパラメータをエディットボックスに反映させる。
	void UpdateParentListBox(); //リストボックスの中身を最新の状態に書き換える(親)
	void UpdateChildListBox(); //リストボックスの中身を最新の状態に書き換える(子)
	void RenameObject();
	HRESULT SeekFile(std::string format); //format拡張子のモデルファイルを探索する

	HRESULT Initialize();
	HRESULT Update();
	HRESULT Terminate();
	HRESULT SaveScene(); //現在のシーンをセーブする
	HRESULT LoadScene(); //保存済みのシーンをロードする
};