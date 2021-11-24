☆3Dモデルパーサーとその描画プログラム

〇動作環境
OS : Windows10
GPU : DirectX12対応のものなら何でもよい

〇操作方法
・\ObjectViewer\ObjectViewerD3D12\Model\OBJディレクトリ直下に、OBJファイル(.obj)を配置
・\ObjectViewer\Debug\ObjectViewerD3D12.exeを起動
・"Editor"ウィンドウにて、最上部のドロップダウンボックスより読み込みたいOBJファイルを選択
・横にある"Load"ボタンを押すことでモデルロード開始、完了次第"Object_Viewer"ウィンドウに3Dモデルが描画される
・下のドロップダウンボックスを開くと、現在ロード済みのモデルが列挙されているので、任意のオブジェクトを選択
・下のスライドバーを調整することで、モデルの姿勢およびサイズの変更可能
　-Position : モデルの位置　    上からx,y,zに対応
　-Rotation : モデルの回転    　上からRall,Pitch,Yawに対応
　-size     : モデルの拡大/縮小 上からx,y,zに対応
・不要になったモデルは横の"Delete"ボタンを押すことで削除可能
・"Object_Viwer"ウィンドウでは、WASDでカメラ移動、マウスを右クリックしながら動かすことで視点移動が可能

詳しくは\ObjectViewer\Demo.mp4を参照