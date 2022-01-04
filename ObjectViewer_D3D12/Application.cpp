#pragma once
#include"Application.h"

Application::Application() {
}

Application::~Application() {

}

HRESULT Application::Initialize() {
	HRESULT hr;

	//ロード可能なモデルファイルを見つけておく
	PathController pc;
	char objsPath[MAX_PATH_LENGTH];

	pc.AddLeafPath(pc.basePath, objsPath, "\\ObjectViewer_D3D12\\Model\\OBJ\\*.obj");

	HANDLE hFind;
	WIN32_FIND_DATA fd;
	hFind = FindFirstFile(objsPath, &fd);
	if (hFind == INVALID_HANDLE_VALUE) {
	}
	else {
		char objFilesPath[MAX_PATH_LENGTH];
		pc.AddLeafPath("\\ObjectViewer_D3D12\\Model\\OBJ\\", objFilesPath, fd.cFileName); //Create Object File Path for program
		DefaultObjFilePaths.push_back(objFilesPath);
		while (FindNextFile(hFind, &fd)) {
			pc.AddLeafPath("\\ObjectViewer_D3D12\\Model\\OBJ\\", objFilesPath, fd.cFileName);
			DefaultObjFilePaths.push_back(objFilesPath);
		}
	}

	//メインウィンドウの生成
	m_mwc = new MainWindowController(hInst, window_Width, window_Height);
	m_mwc->InitWindow("Main", "Main");
	//リストウィンドウの生成
	m_lwc = new ListWindowController(hInst, 500, 1000);
	m_lwc->InitWindow("List", "List");
	m_lwc->InitChildWindow();
	//エディタウィンドウの生成
	m_ewc = new EditWindowController(hInst, 500, 350);
	m_ewc->InitWindow("Editor", "Editor");
	
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

}

HRESULT Application::Update() {
	//1. 入力情報の更新
	input.update();

	//2. リストボックスの更新(newとDelete)
	UpdateListBox();

	//3. エディットボックスの更新(現在選択されているモデルの情報を反映)
	UpdateEditBox();

	//3. カメラ(ビュー行列)の更新
	camera.update(XMFLOAT3(input.inputKey[KEY_D] * (-1) + input.inputKey[KEY_A], input.inputKey[VK_LSHIFT] * (input.inputKey[KEY_W] * (-1) + input.inputKey[KEY_S]), (1 - input.inputKey[VK_LSHIFT]) * (input.inputKey[KEY_W] * (-1) + input.inputKey[KEY_S])), XMFLOAT3(0, input.dPos.x, input.dPos.y), input.inputKey[VK_RBUTTON]);

	//4. 選択されているオブジェクトの更新(位置姿勢サイズ、マテリアルカラー)
	if (m_ewc->m_editFlag) {
		if (m_lwc->m_parentIdx != -1) DxCon.m_objsOBJ[m_lwc->m_parentIdx]->UpdateTransform(m_ewc->m_edValue);
		if (m_lwc->m_childIdx != -1) DxCon.m_objsOBJ[m_lwc->m_parentIdx]->m_obj[m_lwc->m_childIdx]->UpdateMaterial(m_ewc->m_edValue);
		light.updateColor(m_ewc->m_edValue);
		light.update(m_ewc->m_edValue);
		m_ewc->m_editFlag = false;
	}

	//5. ライト視点からのレンダリング(シャドウマップ用)
	if (FAILED(DxCon.DrawFromLight(light))) {
		return ErrorMessage("Failed to Update");
	}

	//5. カメラ視点からのレンダリング(マルチパス用<-今回はやってない)
	if (FAILED(DxCon.DrawFromCamera(camera,light))) {
		return ErrorMessage("Failed to Update");
	}

	//6. 最終的なレンダリング結果をバックバッファにレンダリングし、ウィンドウへ表示する
	if (FAILED(DxCon.finalDraw())) {
		return ErrorMessage("Failed to Update");
	}
}

void Application::UpdateListBox() {
	//モデルのロード
	if (m_lwc->m_isLoad) {
		if (m_lwc->m_loadIdx != -1) {
			if (FAILED(DxCon.LoadObject(m_lwc->m_loadableFileList[m_lwc->m_loadIdx].c_str()))) {
				ErrorMessage("Failed to LoadObject");
			}
			else {
				//リストボックスの更新
				UpdateParentListBox();
			}

		}
		m_lwc->m_isLoad = false;
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

HRESULT Application::Terminate() {
	DxCon.Release();

	return S_OK;
}