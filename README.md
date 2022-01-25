# ☆ObjectViewer

## 〇実行手順

**"\ObjectViewer\Debug\ObjectViewer_D3D12.exe"**実行ファイルより実行してください。

**"\ObjectViewer\ObjectViewer_D3D12"**フォルダ内にソースコードがあります(".h", ".cpp", ".hlsl", ".hlsli")。

なお、同フォルダ内の**"DirectXTex-master"**以下のものは、DirectX12にてテクスチャを簡単に扱うために外部から導入したものになりますので、私が制作したソースコードではありません。

**"\ObjectViewer\ObjectViewer_D3D12\Model\OBJ"**フォルダ内には実際にアプリケーションで読み込むための3Dモデルファイル(.obj)があります。同フォルダ内にwavefront社のOBJフォーマットで記述されているファイルを置けば、アプリケーションで読み込み可能となります。

なお、初期状態で用意しているモデルは**"Floor.obj"**を除いて、https://free3d.com/ja/3d-models/obj より無料でダウンロード可能なものを使用しています。


cpp/hファイルが多いので、以下にそれぞれのファイルについて簡単に役割を示します。

なお、今回は一つのcpp/hファイルに一つのクラスという形をとっています。

cppファイルとhファイルは必ず1:1対応するように同じ名前を付けていますので、「〇〇.cpp/h」という書き方は、「〇〇.cppと〇〇.h」をまとめた表現になります。

また、本アプリケーションは**Flatbuffers**を使用しています。

### ・main.cpp
アプリケーションのエントリー関数があります。
アプリケーションの初期化、更新、終了の処理を行います。
アプリケーションのループを行い、ループの中で、WindowsOSからメッセージの取得及びウィンドウプロシージャ関数の呼び出し、アプリケーションの更新関数の呼び出しを行います。

### ・Application.cpp/h
アプリケーションの全体的な管理を行うクラスです。
メンバ関数に初期化や更新、終了処理を記述したものを持ち、それらはmain関数で呼び出されます。
メンバ変数にウィンドウ管理クラスやDirectX管理クラスの実態を持ち、それらの一部のメンバ関数を呼び出す役割を持ちます。

### ・DirectXController.cpp/h
DirectXの全体的な管理を行うクラスです。
ただし、DirectXの処理を簡略化するようなラッパークラスではありません。あくまでDirectXに深く関連する動作をクラス化したものになります(どちらかというとプログラムの処理の流れをわかりやすくする目的が強いです)。
DirectX12インターフェースのセットアップ関数や実際の描画関数をメンバとして持ちます。
描画の流れとして
ライト視点からの描画(シャドウマップ用)
↓
カメラ視点から描画し、結果をテクスチャデータとして保持(現在は実装していないが、今後ポストエフェクトをやるときのため)
↓
テクスチャデータをウィンドウに表示(ウィンドウサイズのポリゴンにそのままテクスチャを張り付ける処理を行っています)

### ・Camera.cpp/h  Light.cpp/h
それぞれカメラ(= 実際にウィンドウに3D空間を表示する際の視点情報)、ライト(= シャドウイング及びシェーディングを行う際のライト情報)を管理します。

### ・DX12Object3D.cpp/h
DirectX12における3Dポリゴンの扱いを管理するクラスです。
頂点バッファやその記述子(DirectXでビューと呼ばれるものです)等、DirectXの描画に必要なデータを管理します。
なお、当初の開発ではテクスチャの貼り付けを行っていましたが、開発を続けていくうちに今回は不要でよいという結論に至ったため、テクスチャバッファは使用しておりません。

### ・DX12ObjectFormatOBJ.cpp/h
wavefrontのOBJモデルファイルフォーマットに対応するためのクラスです。
モデルファイルをパースし、DirectX12で使用可能なデータに変換する役割を持っています。splitBlank関数やsplitSlash関数はパースのための関数になります。
メンバにDX12Objject3Dクラスの実体を配列で持ちます。これはOBJファイルのメッシュデータを、マテリアル適用位置ごとに分割し別々のポリゴンとして扱うためです。

### ・DX12Shader.cpp/h
DirectX12で扱うシェーダのラッパークラスです。
シェーダのバイナリコードを格納するためのオブジェクトをメンバとして持ちます。
「hlslファイルをコンパイルして読み込むか、見つからなければcsoファイルを探し直接読み込む」という処理をラップしています。

### ・Input.cpp/h
ユーザからの入力を管理するクラスです。
アプリケーションの毎回の更新時に、ユーザが入力しているキーを調べ、そのキーに対応するフラグを立て、他のオブジェクトからの参照を待ちます。
ただし、ユーザからテキストボックス等への入力処理はWin32APIを完全に頼っているので、そちらの実装はしていません。

### ・〇〇WindowController.cpp/h
Win32APIにおけるウィンドウをラップしたクラスです。
BaseWindowController.cpp/hは親クラスとして用意しています。本アプリケーションで表示されるウィンドウはこのクラスを継承します。
MainWindowController.cpp/hは、描画結果を表示するためのウィンドウを管理するクラスです。
EditWindowController.cpp/hは、3D空間におけるいくつかのパラメータを制御するためのウィンドウを管理するクラスです。
ListWindowController.cpp/hは、3Dモデルを列挙するためのウィンドウを管理するクラスです。
RenameWindowController.cpp/hは、3Dモデルの名前を変更するためのウィンドウを管理するクラスです。

### ・PathController.cpp
ファイルパスを管理するためのクラスです。
当初、開発に慣れておらず、相対パス指定が原因でVisualStudio上でのデバッグでは動き、ビルドした.exeファイルでは動作しないという事態に見舞われました。それらの問題に対処するために、実行ファイルのカレントディレクトリを取得し、実行ファイルの位置に依存せずに動作するプログラムとするためにこのクラスを作りました。

### ・Object.cpp/h
初期のアプリケーションプログラムにて使用していたクラスです。
3Dモデル管理クラスとして用意し使用していましたが、コードがあまりきれいでなく、まとまりもなかったため、一度コードのリファクタリングを行う際に、その機能の一部を"DX12Object3D.cpp/h"および"DX12ObjectFormatOBJ.cpp/h"に引き継ぎ、使用を廃止しました。
プログラムの動作には影響ないものとなっていますので、消そうか悩んでいましたが念のため残しています。

### ・ErrorDebug.cpp/h
ソースコード全体として、Windowsプログラムなので、返り値にHRESULT型を多用しています。特に処理に失敗したときは基本的にHRESULT型の定数E_FAILを返していたので、エラー内容の表示とE_FAILのリターンを行うための処理をひとまとめにしたクラスを用意しました。

### ・ConstValue.h
ウィンドウサイズ(横幅と縦幅)が定義されています

### ・BasicShader.hlsli
カメラ視点からのレンダリングに使う、シェーダ用のヘッダファイルです。

### ・FinalRenderShader.hlsli
カメラ視点からのレンダリング結果を、四角形ポリゴンに張り付けて表示するのに使う、シェーダ用のヘッダファイルです。