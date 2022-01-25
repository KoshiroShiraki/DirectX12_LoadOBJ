#pragma once
#include"Application.h"

Application::Application() {
}

Application::~Application() {

}

HRESULT Application::Initialize() {
	HRESULT hr;

	//ロード可能なモデルファイルを見つけておく
	SeekFile("obj");
	SeekFile("fmd");
	for (int i = 0; i < DefaultObjFilePaths.size(); i++) {
		std::cout << DefaultObjFilePaths[i] << std::endl;
	}

	//メインウィンドウの生成
	m_mwc = new MainWindowController(hInst, window_Width, window_Height);
	m_mwc->InitWindow("Main", "Main");
	//リストウィンドウの生成
	m_lwc = new ListWindowController(hInst, 500, 800);
	m_lwc->InitWindow("List", "List");
	m_lwc->InitChildWindow();
	UpdateComboBox(); //中身を更新
	//エディタウィンドウの生成
	m_ewc = new EditWindowController(hInst, 500, 350);
	m_ewc->InitWindow("Editor", "Editor");
	//リネームウィンドウの生成
	m_rwc = new RenameWindowController(hInst, 500, 100);
	m_rwc->InitWindow("Rename", "Rename");
	EnableWindow(m_rwc->m_hwnd, FALSE); //最初は使わないので無効かつ不可視にしておく
	ShowWindow(m_rwc->m_hwnd, SW_HIDE);

	//ウィンドウを表示する
	ShowWindow(m_mwc->m_hwnd, SW_SHOW);
	ShowWindow(m_lwc->m_hwnd, SW_SHOW);
	ShowWindow(m_ewc->m_hwnd, SW_SHOW);

	//DirectXインタフェースの初期化
	if (FAILED(DxCon.InitD3D(m_mwc->m_hwnd))) {
		return ErrorMessage("Failed to Init3D");
	}
	if (FAILED(DxCon.CreateRenderResources())) {
		return ErrorMessage("Failed to CreateRenderResources");
	}
	if (FAILED(DxCon.CreateConstBuffers(camera, light))) {
		return ErrorMessage("Failed to CreateConstBuffers");
	}
	if (FAILED(DxCon.CreateShaders())) {
		return ErrorMessage("Failed to CreateShaders");
	}
	if (FAILED(DxCon.CreateShadowMapGraphicsPipeLine())) {
		return ErrorMessage("Failed to CreateShadowMapGraphicsPipeline");
	}
	if (FAILED(DxCon.CreateFinalGraphicsPipeLine())) {
		return ErrorMessage("Failed to CreateFinalGraphicsPipeLine");
	}
	if (FAILED(DxCon.CreateGraphicsPipeLine())) {
		return ErrorMessage("Failed to Create GraphicsPipeline");
	}

	return S_OK;
}

HRESULT Application::Update() {
	//1. リストボックスの更新(newとDelete)
	UpdateListBox();

	//2. エディットボックスの更新(現在選択されているモデルの情報を反映)
	UpdateEditBox();

	//3. リネームボタンが押されたら、リネームウィンドウの表示
	RenameObject();

	//4. セーブボタンが押されたとき
	if (m_lwc->m_isSave) {
		if (m_lwc->m_parentIdx != -1) {
			DxCon.m_objsOBJ[m_lwc->m_parentIdx]->SaveFMDFile();

			//セーブが行われたので、モデルパス配列とコンボボックスを更新する
			DefaultObjFilePaths.clear();
			DefaultObjFilePaths.shrink_to_fit();
			SeekFile("obj");
			SeekFile("fmd");

			UpdateComboBox();
		}

		m_lwc->m_isSave = false;
	}

	//5. 入力情報の更新(リネーム中でなければ)
	if(!m_lwc->m_isRename) input.update();


	//6. カメラ(ビュー行列)の更新
	camera.update(XMFLOAT3(input.inputKey[KEY_D] * (-1) + input.inputKey[KEY_A], input.inputKey[VK_LSHIFT] * (input.inputKey[KEY_W] * (-1) + input.inputKey[KEY_S]), (1 - input.inputKey[VK_LSHIFT]) * (input.inputKey[KEY_W] * (-1) + input.inputKey[KEY_S])), XMFLOAT3(0, input.dPos.x, input.dPos.y), input.inputKey[VK_RBUTTON]);

	//7. 選択されているモデルの更新(位置姿勢サイズ、マテリアルカラー)
	if (m_ewc->m_editFlag) {
		if (m_lwc->m_parentIdx != -1) DxCon.m_objsOBJ[m_lwc->m_parentIdx]->UpdateTransform(m_ewc->m_edValue);
		if (m_lwc->m_childIdx != -1) DxCon.m_objsOBJ[m_lwc->m_parentIdx]->m_obj[m_lwc->m_childIdx]->UpdateMaterial(m_ewc->m_edValue);
		light.updateColor(m_ewc->m_edValue);
		light.update(m_ewc->m_edValue);
		m_ewc->m_editFlag = false;
	}

	//8. ライト視点からのレンダリング(シャドウマップ用)
	if (FAILED(DxCon.DrawFromLight(light))) {
		return ErrorMessage("Failed to DrawFromLight");
	}

	//9. カメラ視点からのレンダリング(マルチパス用<-今回はやってない)
	if (FAILED(DxCon.DrawFromCamera(camera,light))) {
		return ErrorMessage("Failed to DrawFromCamera");
	}

	//10. 最終的なレンダリング結果をバックバッファにレンダリングし、ウィンドウへ表示する
	if (FAILED(DxCon.finalDraw())) {
		return ErrorMessage("Failed to finalDraw");
	}
}

void Application::UpdateComboBox() {
	//ListBoxの中身をリセット
	SendMessage(m_lwc->m_chwnd, CB_RESETCONTENT, 0, 0);

	PathController pc;

	//DefaultObjFilePathsからファイル名 + フォーマット　を抽出して表示
	for (int i = 0; i < DefaultObjFilePaths.size(); i++) {
		char name[MAX_PATH_LENGTH];
		pc.GetLeafDirectryName(DefaultObjFilePaths[i].c_str(), name, MAX_PATH_LENGTH);
		std::cout << name << std::endl;
		SendMessage(m_lwc->m_chwnd, CB_ADDSTRING, 0, (LPARAM)name);
	}
}

void Application::UpdateListBox() {
	//モデルのロード
	if (m_lwc->m_isLoad) {
		if (m_lwc->m_loadIdx != -1) {
			if (FAILED(DxCon.LoadObject(DefaultObjFilePaths[m_lwc->m_loadIdx].c_str()))) {
				ErrorMessage("Failed to LoadObject");
			}
			else {
				//リストボックスの更新
				UpdateParentListBox();
			}

		}
		m_lwc->m_isLoad = false;
	}

	//モデルの複製
	if (m_lwc->m_isDuplicate) {
		if (m_lwc->m_parentIdx != -1) {
			DxCon.DuplicateObject(m_lwc->m_parentIdx);
			UpdateParentListBox();
		}
		m_lwc->m_isDuplicate = false;
	}

	//モデルの削除
	if (m_lwc->m_isDelete) {
		if (m_lwc->m_parentIdx != -1) {
			DxCon.DeleteObject(m_lwc->m_parentIdx);
			//削除したら、親/子インデックスを-1(何も選択していない状態)にする
			m_lwc->m_parentIdx = -1;
			m_lwc->m_childIdx = -1;

			//リストボックスの更新
			UpdateParentListBox();
		}
		m_lwc->m_isDelete = false;
	}

	//子モデルの列挙(新しく別のモデルが選択された場合のみ)
	static int pIdx = -1;
	if (m_lwc->m_parentIdx != pIdx) {
		UpdateChildListBox();
	}
	pIdx = m_lwc->m_parentIdx;
	
}

void Application::UpdateEditBox() {
	if (m_lwc->m_isParentChanged) { //親オブジェクトが新しく選択されたとき
		m_ewc->UpdateEditBoxTransform(
			DxCon.m_objsOBJ[m_lwc->m_parentIdx]->m_transform.position, 
			DxCon.m_objsOBJ[m_lwc->m_parentIdx]->m_transform.rotation, 
			DxCon.m_objsOBJ[m_lwc->m_parentIdx]->m_transform.size
		);
		m_lwc->m_isParentChanged = false;
	}
	if (m_lwc->m_isChildChanged) { //子オブジェクトが新しく選択されたとき
		m_ewc->UpdateEditBoxMaterial(
			DxCon.m_objsOBJ[m_lwc->m_parentIdx]->m_obj[m_lwc->m_childIdx]->m_material.ambient, 
			DxCon.m_objsOBJ[m_lwc->m_parentIdx]->m_obj[m_lwc->m_childIdx]->m_material.diffuse, 
			DxCon.m_objsOBJ[m_lwc->m_parentIdx]->m_obj[m_lwc->m_childIdx]->m_material.specular,
			DxCon.m_objsOBJ[m_lwc->m_parentIdx]->m_obj[m_lwc->m_childIdx]->m_material.N
		);
		m_lwc->m_isChildChanged = false;
	}
}

void Application::UpdateParentListBox() {
	SendMessage(m_lwc->m_lhwnd[0], LB_RESETCONTENT, 0, 0);
	for (int i = 0; i < DxCon.m_objsOBJ.size(); i++) {
		SendMessage(m_lwc->m_lhwnd[0], LB_ADDSTRING, 0, (LPARAM)DxCon.m_objsOBJ[i]->m_name.c_str());
	}
}

void Application::UpdateChildListBox() {
	SendMessage(m_lwc->m_lhwnd[1], LB_RESETCONTENT, 0, 0);
	if (m_lwc->m_parentIdx != -1) {
		for (int i = 0; i < DxCon.m_objsOBJ[m_lwc->m_parentIdx]->m_obj.size(); i++) {
			SendMessage(m_lwc->m_lhwnd[1], LB_ADDSTRING, 0, (LPARAM)DxCon.m_objsOBJ[m_lwc->m_parentIdx]->m_obj[i]->m_name.c_str());
		}
	}
}

void Application::RenameObject() {
	//リネームボタンが押されたら、リネームウィンドウの表示
	if (m_lwc->m_isRename) {
		if (!IsWindowVisible(m_rwc->m_hwnd)) {
			EnableWindow(m_rwc->m_hwnd, TRUE);
			ShowWindow(m_rwc->m_hwnd, SW_SHOW);
		}

		//リネームウィンドウの完了フラグが立ったら、文字列を参照しモデル名を変更する
		if (m_rwc->m_renameCompleted) {
			DxCon.m_objsOBJ[m_lwc->m_parentIdx]->m_name = m_rwc->name;

			//それぞれのフラグを下げる
			m_rwc->m_renameCompleted = false;
			m_lwc->m_isRename = false;

			//ウィンドウを非表示に
			EnableWindow(m_rwc->m_hwnd, FALSE);
			ShowWindow(m_rwc->m_hwnd, SW_HIDE);

			//リストボックスの更新
			UpdateParentListBox();
		}
	}
}

HRESULT Application::SeekFile(std::string format) {
	PathController pc;
	char path[MAX_PATH_LENGTH];

	pc.AddLeafPath(pc.basePath, path, ("\\ObjectViewer_D3D12\\Model\\OBJ\\*." + format).c_str()); //.format探索用

	//.formatファイルを列挙
	HANDLE hFind;
	WIN32_FIND_DATA fd;
	hFind = FindFirstFile(path, &fd);
	if (hFind == INVALID_HANDLE_VALUE) {
		return ErrorMessage(("There is no" + format + " file"));
	}
	else {
		char objFilesPath[MAX_PATH_LENGTH];
		pc.AddLeafPath("\\ObjectViewer_D3D12\\Model\\OBJ\\", objFilesPath, fd.cFileName);
		DefaultObjFilePaths.push_back(objFilesPath);
		while (FindNextFile(hFind, &fd)) {
			pc.AddLeafPath("\\ObjectViewer_D3D12\\Model\\OBJ\\", objFilesPath, fd.cFileName);
			DefaultObjFilePaths.push_back(objFilesPath);
		}
	}
	return S_OK;
}

HRESULT Application::Terminate() {
	DxCon.Release();

	return S_OK;
}