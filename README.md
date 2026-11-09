# SHEngine 使い方ガイド

このドキュメントでは、`SHEngine` (DirectX12向けゲームエンジン) の基本的な使い方について解説します。
エンジン本体は `DirectXGame/SHEngine/` にあり、`DirectXGame/AppSource/` に利用例があります。


## 目次
0. [はじめに（外部ライブラリのセットアップ）](#0-はじめに外部ライブラリのセットアップ)
1. [Windowの作成方法](#1-windowの作成方法)
2. [コマンドコンテキストについて](#2-コマンドコンテキストについて)
3. [Texture, Modelの読み込み](#3-texture-modelの読み込み)
4. [描画の方法](#4-描画の方法)
5. [ComputeObjectについて](#5-computeobjectについて)
6. [Displayについて (OffScreen)](#6-Displayについて)
7. [Utilityについて](#7-utilityについて)
8. [Toolについて](#8-toolについて)
9. [ImGuiについて](#9-imguiについて)
10. [応用機能](#10-応用機能)


---

## 0. はじめに（外部ライブラリのセットアップ）

開発を始める前に、必ず `Scripts/setup.bat` を起動して外部ライブラリをコンパイルしてください。
ライブラリ関係はソースコードのまま保管されているため、これを実行しないとヘッダーファイル (.h) や静的ライブラリ (.lib) の不足によるエラーが発生します。

`Scripts/setup.bat` を正常に実行するためには、CMakeが必要です。

- **CMake (4.2以上)**
  付属の `Scripts/CMakeRegister.bat` を実行することでインストールできます。
  ※環境変数を書き換えるため、**必ず管理者権限で実行**してください。

また、setup.batを起動時、gitがインストールされていない場合に自動でインストールされます。  
batの再起動が必要になる場合があるので、画面の指示に従ってください。

---

## 1. Windowの作成方法

WindowsAPIとSwapChainを組み合わせて描画可能なWindowを作成します。
エンジンの初期化時に利用する主なフローは以下の通りです。

```cpp
// 1. WindowsAPIオブジェクトの作成
auto windowsAPI = std::make_unique<SHEngine::Screen::WindowsAPI>();

// 2. Windowの設定を記述
SHEngine::Screen::WindowsAPI::WindowDesc desc;
desc.width = 1280;
desc.height = 720;
desc.windowName = L"SHEngine";
desc.wndProc = [&](HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) -> LRESULT {
#ifdef USE_IMGUI
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) return true;
#endif
    return DefWindowProc(hwnd, msg, wparam, lparam);
};

// 3. 初期化
windowsAPI->Initialize(desc, engine_->GetHInstance());

// SwapChainを作成。windowの所有権はSwapChainへ移る。
auto swapChain = std::make_unique<SHEngine::Screen::SwapChain>();
swapChain->Initialize(engine_->GetTextureManager(), engine_->GetDirectCommandContext(), 0xffffffff, std::move(windowsAPI));
```

作成した `SwapChain` は `DirectCommandContext::SetRenderTarget()` に渡して描画先に設定します。描画ループでは `Engine::BeginFrame()` でフレームを開始し、描画コマンドを発行した後、ImGui を使う場合は `Engine::DrawImGui()` を呼びます。最後に `SwapChain::ToPresent()`、`DirectCommandContext::EndFrame()`、`SwapChain::Present()`、`Engine::PostDraw()` を呼んで画面に表示します。


---

## 2. コマンドコンテキストについて

エンジンは Direct と Compute のコマンドコンテキストを所有しており、`Engine` の Getter から取得します。コンテキスト自体は `Engine::Initialize()` 時に作成されます。

```cpp
// コマンドコンテキストを取得
auto directContext = engine_->GetDirectCommandContext();
auto computeContext = engine_->GetComputeCommandContext();
```

※ `SHEngine::Command::Type::Direct` と `SHEngine::Command::Type::Compute` の違い:
- **Direct**: 描画（グラフィックス）や一般的な処理を行うための標準的なコマンドキュー/リストを使用します。
- **Compute**: コンピュートシェーダーなど、主に汎用計算処理（GPGPU）に特化したキュー/リストを使用します。

コマンドは `DirectCommandContext` または `ComputeCommandContext` を通して記録します。各コンテキストは `BeginFrame()` で開始し、`MiddleExecute()` で途中実行（必要な場合）、`EndFrame()` で残りのコマンドを実行します。`DirectCommandContext` は描画先の設定にも使います。


---

## 3. Texture, Modelの読み込み

アセット関連は `DirectXGame/SHEngine/Assets/` 内の `TextureManager`、`ModelManager`、`FontLoader`、`AudioManager` などが管理しています。主要なマネージャーは `Engine` の Getter から取得できます。

### ModelManager
Assimpを使用して様々な形式の3Dモデルやアニメーションを読み込みます。

```cpp
// モデルを読み込み、ModelDataへのポインタを取得
const SHEngine::ModelData* modelData = modelManager->LoadModel("model.obj");

// Assets/Models/ 以下のモデルをまとめて読み込む場合
modelManager->LoadAllModels();
```

### TextureManager
ファイルからのテクスチャ読み込みや、生成したTextureのGPUへのアップロードを行います。

```cpp
// テクスチャの読み込み（ファイルパスを指定）
int texHandle = textureManager->LoadTexture("texture.png");

// 【重要】
// 読み込んだテクスチャデータを実際にGPU上のリソースとして利用可能にするため、
// まとめてアップロードする工程が必要です。
textureManager->LoadAllTextures();
```



#### AudioManager

`AudioManager` は WAV と MP3 を読み込みます。`Engine::Initialize()` 時に初期化されるシングルトンからデータを取得し、再生します。

```cpp
auto* audio = AudioManager::GetInstance()->GetData("BGM/sample.mp3");
audio->Play(); // 最後まで再生
// auto playing = audio->CustomPlay(0); // 再生状態を個別に操作する場合
AudioManager::GetInstance()->SetVolume(0.5f); // 全体音量 (0.0〜1.0)
```

`AudioData::SetVolume()` は個別データの音量、`PlayData::SetVolume()` は個別再生音の音量を調整します。

---

## 4. 描画の方法

描画には `DirectXGame/SHEngine/Render/` 内の `Renderer`、Mesh Shader 用の `MeshRenderer`、`GPUBuffer` が関わります。頂点・インデックスバッファを使う描画には `Renderer`、Mesh/Amplification Shader を使う描画には `MeshRenderer` を使用します。

### GPUBuffer
定数バッファ(CBV)やシェーダーリソース(SRV)など、GPUへ渡すデータを管理する重要なクラスです。
データのコピーやリソースバリアの変更、フラッシュ（実際のGPU反映）などをラップしています。通常は `BufferContainer` 経由で生成します。

### Renderer を使った描画の例
`DirectXGame/AppSource/` 配下のシーン実装などを参考に、次のように設定します。

```cpp
// 1. 頂点形式を指定してRendererを作成
auto renderer = std::make_unique<SHEngine::Renderer>(SHEngine::VertexType::Position);

// 2. シェーダーの設定
renderer->SetVS("Custom.VS.hlsl");
renderer->SetPS("Custom.PS.hlsl");

// 3. GPUBufferの作成とセット
auto container = std::make_unique<SHEngine::BufferContainer>();
auto vsBuffer = container->Create(BufferType::SRV, sizeof(VSData), instanceNum);
auto psBuffer = container->Create(BufferType::CBV, sizeof(PSData));

renderer->SetGPUBuffer(vsBuffer, SHEngine::ShaderType::VERTEX_SHADER, BufferType::SRV);
renderer->SetGPUBuffer(psBuffer, SHEngine::ShaderType::PIXEL_SHADER, BufferType::CBV);
renderer->instanceNum_ = instanceNum;

// 毎フレームの描画処理
// DirectCommandContextを渡してドローコールを発行
renderer->Draw(directContext);
```


---

## 5. ComputeObjectについて

コンピュートシェーダー(ComputeShader)の実行を簡単に行うためのクラスが `ComputeObject` です。

### 特徴と使い方
- `Renderer` と同じく、`GPUBuffer` を扱えます。
- 実行グループ数は `SetExecuteNum()` または `SetThreadGroupSize()` で指定します。UAV の計算結果は、リソース状態を適切に遷移させた後、描画側で SRV として利用できます。

```cpp
SHEngine::ComputeObject computeObj("MyCompute");

// 1. シェーダーのファイル名をセット
computeObj.SetShader("MyCompute.CS.hlsl");

// 2. Dispatchするスレッドグループ数を設定
computeObj.SetThreadGroupSize(32, 32, 1);

// 3. GPUBuffer (UAVやSRV, CBVなど) をセット
computeObj.SetGPUBuffer(BufferType::UAV, myUavBuffer);

// 4. 実行 (ComputeCommandContextを渡してディスパッチ)
computeObj.Execute(computeContext);
```
※ `Execute()` はコマンドをコンテキストに記録します。`computeContext->EndFrame()` または必要に応じて `MiddleExecute()` を呼ぶとキューに投入されます。


---

## 6. Displayについて (OffScreen)

メインのウィンドウとは別に、オフスクリーンレンダリング(マルチパスなど)を行いたい場合は `Display` を使用します。

### 使い方
```cpp
auto display = std::make_unique<SHEngine::Screen::Display>();
display->Initialize(1280, 720, "OffScreen");
display->CreateDepthTexture(textureManager);
display->AddRenderTarget(textureManager, 0x050505ff);

// --- 描画ループ内 ---

// 1. ターゲットをこのDisplayに変更し、画面をクリア
directContext->SetRenderTarget(display.get());

// ... (ここで Renderer 等を使って描画) ...

// 2. 描画結果をテクスチャとして使える状態へ移行
display->ToTexture(directContext);

// その後、display->GetTextureData() で取得したTextureを描画用のSRVとして利用可能
```


---

## 7. Utilityについて

`DirectXGame/SHEngine/Utility/` にはよく使われる計算系の構造体が含まれています。

- **Vector2, Vector3, Vector4**:
  よくある要素数と同数のfloatを持つベクトル型です。四則演算のオペレーターオーバーロードなどが備わっています。
- **Matrix**:
  `MatrixFactory` 等と組み合わせて、平行移動・回転・拡縮などの行列計算を簡単に行うことができます。
- **Easing**:
  非線形な補間（イージング）の計算式がまとまっており、アニメーションやUIの動きなどに利用します。


---

## 8. Toolについて

ゲーム開発における「最低限のサポート」機能群です。

- **BinaryManager**:
  ファイル出力/読み込み(バイナリデータ化)を簡易に行うツールです。エディタで作った配置データの保存、ステージパラメータの管理などに利用できます。
- **Logger**:
  標準出力やファイルに対してログを吐き出します。`GetLogger("ModuleName")` のようにモジュールごとに分けてログを取る仕組みがあります。
- **Grid**:
  デバッグ用の3Dグリッドなどを描画するためのシンプルなユーティリティです。空間を把握する際に手軽に導入可能です。
- **JsonManager**: `Assets/Json/` 配下の JSON を `Boot()` で読み込み、`Get<T>()` / `Add()` / `Save()` で扱います。Vector、Matrix、Quaternion にも対応しています。
- **TimeViewer**: 名前付きの処理時間をミリ秒表示する計測用ツールです。`Initialize(engine)` の後、`Add(name, time)` に秒単位の時間を渡して登録・更新し、描画時に `Draw(directContext)` を呼びます。`DrawImGui()` では表示倍率、行間、位置、色を調整できます。
- **FPSObserver**: CPU/GPU の FPS 固定とデルタタイム取得に使います。Engine からは `GetFPSObserver()` で取得できます。
- **DecoEditor**: Display 上でオブジェクト配置データを編集し、Undo/Redo や選択オブジェクト操作を提供します。


---

## 9. ImGuiについて

`ImGui` によって、デバッグ機能などのGUIを簡単にアプリ上に合成できます。
エンジン自体にWrapperが含まれており、以下のように組み込みます。

```cpp
// 1. Windowsのプロシージャにて ImGui のハンドラを仕込む (前述のWindow作成時)

// 2. ImGuiの有効化
engine_->ImGuiActivate(windowsAPI.get());

// --- アプリのUpdate中など ---
ImGui::Begin("Debug Window");
ImGui::Text("Hello SHEngine!");
ImGui::End();

// SwapChainを描画先に設定した後、Engine::DrawImGui()を呼びます
engine_->DrawImGui();
```

上記手順により、ユーザー側は `ImGui::Begin` 〜 `ImGui::End` を自由に呼び出すだけでGUIが作成できます。

---

## 10. 応用機能

### Mesh Shader 描画

`MeshRenderer` は Mesh Shader と任意の Amplification Shader を使った描画を行います。シェーダー、PSO 設定、CBV/SRV/UAV を指定し、Dispatch group 数を設定して描画します。

```cpp
SHEngine::MeshRenderer meshRenderer;
meshRenderer.SetAS("Particle/Quad.AS.hlsl"); // Amplification Shaderが不要なら省略
meshRenderer.SetMS("Particle/Quad.MS.hlsl");
meshRenderer.SetPS("Game/GPUParticle.PS.hlsl");
meshRenderer.SetGPUBuffer(particleBuffer, ShaderType::MESH_SHADER, BufferType::SRV);
meshRenderer.SetDispatchGroup(groupCount);
meshRenderer.Draw(directContext);
```

バッファの `ShaderType` は各シェーダーのレジスタ定義に合わせます。Mesh Shader の PSO は `MeshRenderer` 側で管理され、通常の `Renderer` と設定項目が一部異なります。

### ポストエフェクト

`PostEffect` は入力 Display の画像をフルスクリーン描画し直し、ピクセルシェーダーで効果を適用します。グレースケール、ビネット、ボックス／ガウスブラー、エッジ検出、アウトライン、ラジアルブラー、ディゾルブ、フェードに対応します。各ジョブの設定用構造体を `CopyBuffer()` で渡し、`PostEffectConfig::jobs` に適用する効果をビットフラグで指定します。複数の効果を指定した場合は順に適用されます。`output` を省略（`nullptr`）すると入力の `origin` に描き戻し、別の Display を指定するとそこへ最終結果を出力します。`Initialize()` が用意する中間 Display は 1280×720 なので、入力・出力も同じ解像度で使う想定です。

```cpp
PostEffect effect;
effect.Initialize(engine_->GetTextureManager());

Grayscale grayscale;
grayscale.intensity = 0.8f;
effect.CopyBuffer(PostEffectJob::GrayScale, grayscale);

PostEffectConfig config;
config.dcc = directContext;
config.origin = sceneDisplay;    // 効果をかける入力
config.output = nullptr;       // nullptrならoriginへ出力
config.jobs = uint32_t(PostEffectJob::GrayScale); // 複数指定は PostEffectJob::GrayScale | PostEffectJob::Fade
effect.Draw(config);
```

### カメラとカメラカーブ編集

`Camera` は透視投影と正射影に対応し、位置・回転・投影設定から View-Projection 行列を作ります。`DebugCamera` は入力を使って注視点の周りを操作するデバッグ用カメラです。`CameraEditor` と `CameraCurveData` を使うと、位置・回転のキーをカーブとして編集・再生できます。`CameraCurveData` は `BinaryManager` で保存・読み込みできます。

### 衝突判定

`ColliderManager` は登録された Collider を検査し、衝突時に `OnCollision()`、非衝突時に `NonCollision()` を呼びます。形状は `Circle`、`Quad`、扇形の `DirCircle`、タグは `CollTag` のビットフラグで指定します。Collider を使う派生クラスを作成し、判定対象タグと形状を設定した後、ゲーム側の更新処理から `CollisionCheckAll()` を呼びます。ColliderManager は Collider の登録前に `Collider::SetColliderManager()` で設定してください。

### テキスト描画

`FontLoader` は FreeType を使ってフォントを読み込み、`Text` は文字列を描画します。フォントは `Assets/Fonts/` を基準に指定し、初期化時に使用する平面 Mesh とフォントサイズを渡します。

```cpp
SHEngine::Text text;
text.Initialize(planeMesh, "MPLUS1p-Medium.ttf", 64);
text.SetText("Hello SHEngine");
text.SetSize(1.0f);
text.SetColor({1, 1, 1, 1});
text.Update(camera.GetVPMatrix());
text.Draw(directContext);
```
