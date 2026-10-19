# AI Review

レビュー対象: `DirectXGame/SHEngine` 配下のエンジン自前 C++ ヘッダー／実装。`externals`、生成物、依存ライブラリは除外。以下はコードを変更せずに列挙した改善候補です。行番号は確認時点の目安です。

## スペル・命名

| 優先度 | 箇所 | 指摘 |
|---|---|---|
| 低 | `DirectXGame/SHEngine/Tool/FPS/FPSObserver.h:57`, `DirectXGame/SHEngine/SHEngine.h:60` | `GetDeltatime` は `GetDeltaTime` の綴りに統一する。呼び出し元があるため、変更時は参照先もまとめて更新する。 |
| 低 | `DirectXGame/SHEngine/Render/Screen/Display.h:68`, `DirectXGame/SHEngine/Render/Screen/Display.cpp:261` | `IsForcus` / `isForcus_` は `IsFocused` / `isFocused_` の意図と思われる。公開API名の変更になるため利用箇所を同時に直す。 |
| 低 | `DirectXGame/SHEngine/Tool/ModelDrawer/ModelDrawer.h:42` | `IsWireFrame(bool)` は状態を設定する関数なので `SetWireframe(bool)` など、動作が分かる名前が適切。引数 `isWireFrame` が未使用で、現状は常に Wireframe を設定する。 |

## `const` を付けられる可能性が高い関数

以下は宣言と実装・既知のクラス状態から、オブジェクトを変更しない読み取り関数と判断できる候補です。ポインタ／参照の指す先を変更できるかというAPI設計も確認してから `const` 化してください。

| 優先度 | 箇所 | 候補 |
|---|---|---|
| 中 | `DirectXGame/SHEngine/SHEngine.h:53-60` | `GetTextureManager`, `GetFontLoader`, `GetModelManager`, `GetDirectCommandContext`, `GetComputeCommandContext`, `GetInput`, `GetFPSObserver`, `GetDeltaTime` はメンバーを読むだけに見えるため末尾 `const` を追加可能。const オブジェクトからも呼べるAPIにするなら、const overload と返却ポインタのconst性も検討する。 |
| 中 | `DirectXGame/SHEngine/Core/DXDevice.h:57,61,65,69,73` | `GetDevice`, `GetDxgiFactory`, `GetSRVManager`, `GetRTVManager`, `GetDSVManager` は所有ハンドルの取得だけであれば末尾 `const` を追加可能。 |
| 中 | `DirectXGame/SHEngine/Core/Command/ICommandContext.h:23,32,34,48` | `GetCommandQueue`, `GetCommandList`, `GetCurrentID`, `GetCurrentCmdObj` は参照中の状態を変更しないアクセサに見えるため末尾 `const` の候補。 |
| 中 | `DirectXGame/SHEngine/Render/Command/DirectCommandContext.h:15` | `GetRenderTarget` はポインタ取得のみのため末尾 `const` の候補。 |
| 中 | `DirectXGame/SHEngine/Assets/Texture/TextureManager.h:110,117,124` | `GetTextureData` 2種と `GetTextureResource` は検索／取得処理を確認し、マネージャー状態を変えないなら末尾 `const` が可能。値引数 `std::string filePath` は `const std::string&` にすると不要なコピーも避けられる。 |
| 中 | `DirectXGame/SHEngine/Assets/Audio/AudioManager.h:29` | `GetData` は読み取りのみなら末尾 `const`。`std::string filePath` は `const std::string&` に変更してコピーを避けられる。 |
| 低 | `DirectXGame/SHEngine/Render/Screen/IDisplay.h:44,47,53,62,65,69` および `Display.h` の override | フォーマット・個数・位置・サイズ・カーソル位置取得は読み取り専用。基底の仮想宣言と全 override を揃えて末尾 `const` にできる。 |
| 低 | `DirectXGame/SHEngine/Render/Screen/Display.h:67-68` | `IsHovering` と `IsForcus` は状態参照のみなら末尾 `const` にできる（実装も確認）。 |
| 低 | `DirectXGame/SHEngine/Tool/FPS/FPSObserver.h:57,59` | `GetDeltatime` と `GetCPUTime` は配列・値の読み出しだけなので末尾 `const` にできる。 |
| 低 | `DirectXGame/SHEngine/Tool/DecoEditor/Data/DecoDataManager.h:34` など既に `const` のアクセサ | `const` 化済みのAPIもあるため、上記の同種アクセサと整合させる。 |

## 設計・効率化の候補

| 優先度 | 箇所 | 改善案と根拠 |
|---|---|---|
| 高 | `DirectXGame/SHEngine/Render/PSO/PSOManagerForMS.h:22-45`, `DirectXGame/SHEngine/Render/PSO/PSOManagerForMS.cpp:10-50` | `ConfigMSType` の `==` / `<` が `as`, `dsvFormat`, `rtvFormats` を比較していない一方、PSO生成ではこれらを利用している。異なる設定が同じキャッシュキーになり得る。キー項目を一元定義した `operator<=>` / hash＋`unordered_map`（または完全な比較）にまとめ、PSO生成に影響する全フィールドを比較する。これは効率化以前にキャッシュ正当性の問題。 |
| 中 | `DirectXGame/SHEngine/Core/View/DSVManager.*`, `RTVManager.*`, `SRVManager.*` | 3クラスでヒープ作成、空きスロット探索、使用フラグ管理、Handle解放が重複している。ヒープ種別や CPU/GPU可否をポリシーとして渡す共通の descriptor allocator / RAII handle に寄せると重複修正を減らせる。種別ごとの探索規則（SRVの折り返し等）は明示的に保つ。 |
| 中 | `DirectXGame/SHEngine/Render/PSO/PSOManager.h`, `PSOManagerForMS.h` | 通常PSOと Mesh Shader PSO がそれぞれキャッシュを持ち、設定構造体や生成フローが分かれている。共通の PSO cache / pipeline descriptor 生成部を抽出し、通常描画と Mesh Shader 固有のサブオブジェクトだけを別ポリシーにすると重複と設定漏れを抑えられる。上記の比較漏れも共通キー検証の対象にする。 |
| 中 | `DirectXGame/SHEngine/Assets/Texture/TextureManager.*`, `Assets/Model/ModelManager.*`, `Assets/Audio/AudioManager.*` | アセットごとの管理処理が増える場合、パス正規化・重複ロード抑止・キャッシュ検索を共通の `AssetCache<T>` / Repository に分け、ロード形式固有部分をローダーにする構成が候補。現状の規模で一律に抽象化するより、重複ロジックが確認できる範囲から適用する。 |
| 低 | `DirectXGame/SHEngine/Render/Screen/IDisplay.h:7-69`, `Display.*`, `SwapChain.*` | `IDisplay` は画面種別の違いを仮想関数で吸収する Strategy 相当の構造をすでに採用している。新しい表示先を追加する場合も個別の型に処理を閉じ込め、利用側で型判定や `dynamic_cast` による分岐を増やさない。既存設計を活かす項目。 |
| 低 | `DirectXGame/SHEngine/Collision/Data/CollisionVisitor.*`, `Collision/ColliderManager.*` | Collider の組み合わせごとに処理が増えるなら、既存の Visitor を組み合わせディスパッチに集約する。ColliderManager側に型ごとの条件分岐が増えた場合の拡張点として記録。 |

## プロジェクトの良いところ

| 観点 | 箇所 | 評価 |
|---|---|---|
| DirectX 12 の複雑さを用途別クラスにまとめている | `DirectXGame/SHEngine/Core/Command/CommandObject.h`, `DirectXGame/SHEngine/Render/Renderer.h`, `DirectXGame/SHEngine/Compute/ComputeObject.h` | コマンドリスト、描画、コンピュートの責務が型として分かれており、利用側が低レベルAPIの手順を毎回組み立てずに済む。 |
| GPUリソースの寿命を所有型で表現している | `DirectXGame/SHEngine/Render/Buffer/BufferContainer.h`, `DirectXGame/SHEngine/Assets/Texture/TextureManager.h`, `DirectXGame/SHEngine/Render/PSO/PSOManager.h` | `unique_ptr` / `ComPtr` を活用し、バッファやテクスチャ、PSOの所有権と解放を管理クラスに寄せている。生ポインタだけで所有権を表す構成より追跡しやすい。 |
| 画面種別を共通インターフェースで扱える | `DirectXGame/SHEngine/Render/Screen/IDisplay.h`, `Display.h`, `SwapChain.h` | オフスクリーン表示とスワップチェーン表示を `IDisplay` として扱い、描画側が表示方式の詳細から分離されている。 |
| 再利用と拡張のための描画設定部品がある | `DirectXGame/SHEngine/Render/PSO/ShelfManager.h`, `Render/PSO/Shelf/` | シェーダー、ブレンド、深度ステンシル、ラスタライザー、ルートシグネチャなどの設定を個別の Shelf に分けており、PSO構築や設定の再利用につなげている。 |
| GPUでの計算結果を描画へつなげられる | `DirectXGame/SHEngine/Compute/ComputeObject.h`, `DirectXGame/SHEngine/Render/Buffer/GPUBuffer.h` | ComputeObject と共通GPUバッファを通じて、計算結果を後続処理で利用する経路が用意されている。レンダリングと汎用計算を同じリソース管理基盤で組み合わせられる。 |
| 開発時の可視化・編集機能をエンジン内に持つ | `DirectXGame/SHEngine/Render/PSO/PSOEditor.h`, `DirectXGame/SHEngine/Camera/Editor/CameraEditor.h`, `DirectXGame/SHEngine/Tool/TimeViewer/TimeViewer.h` | PSOやカメラ、処理時間を実行中に確認・編集する道具があり、反復開発時の調査と調整に役立つ。 |
| 衝突形状の組み合わせを Visitor に分離している | `DirectXGame/SHEngine/Collision/Data/CollisionVisitor.h`, `Collision/ColliderManager.h` | コライダー管理と形状ペアごとの判定ロジックを分ける方向性があり、判定追加の置き場所が明確。 |

## プロジェクトのわるいところ・改善余地

以下は実装上のリスクや保守性の改善余地です。良いところと同様に、特定の問題を指摘する場合は該当箇所を併記しています。

| 優先度 | 箇所 | 指摘 |
|---|---|---|
| 高 | `DirectXGame/SHEngine/Core/View/DSVManager.h`, `RTVManager.h`, `SRVManager.h` および各 Handle | Handle のデストラクタが manager の使用フラグを戻す一方、manager の寿命を生ポインタで参照している。Handle が manager より長生きすると解放時に無効参照となるため、所有順序を保証するか、共有状態を持つ allocator/token 等で寿命を明確にしたい。 |
| 高 | `DirectXGame/SHEngine/Render/PSO/PSOManagerForMS.h:22-45` | PSO設定の比較演算子が生成に使われる一部フィールドを比較していない。詳細は上記「設計・効率化の候補」の項を参照。キャッシュの誤再利用につながる可能性がある。 |
| 中 | `DirectXGame/SHEngine/Render/Screen/IDisplay.h:12,35,44,47,53,59,62,65,69` | 基底インターフェースにグローバル static 状態や空実装の仮想関数、既定値付き仮想関数が混在している。派生型が実装すべき契約と任意機能の境界が曖昧になりやすい。必須操作は純粋仮想、任意機能は明示的な capability/API に分けることを検討する。 |
| 中 | `DirectXGame/SHEngine/Render/Renderer.h:13,60-61`, `DirectXGame/SHEngine/Tool/ModelDrawer/ModelDrawer.h:42` | Renderer の設定先や GPU テクスチャハンドルが static 共有状態になっている。複数Engine/Deviceを同一プロセスで扱う場合や、初期化・終了順序が変わる場合に状態が混線し得る。Engine/Device単位の Context または注入依存に移すと境界が明確になる。Wireframe設定関数も引数が未使用。 |
| 中 | `DirectXGame/SHEngine/Assets/Model/ModelManager.h:52-71`, `DirectXGame/SHEngine/Assets/Texture/TextureManager.h` | Manager が生ポインタの依存参照と所有データを併せ持つ。初期化前利用や依存先より長い寿命の管理を型から検出しにくい。参照メンバー、初期化時の必須依存の明示、Engine所有順序の整理を検討する。 |
| 中 | `DirectXGame/SHEngine/Render/Screen/Display.cpp:16-45,63-81` | Direct3Dリソース生成の失敗を `assert` のみで扱う箇所がある。Releaseビルドでは assert が無効になり得るため、失敗後に無効なリソースを使う可能性がある。HRESULTを呼び出し元へ返すか、例外・エラー結果として伝播させる方針を統一する。 |
| 低 | `README.md` および `DirectXGame/SHEngine` の実際の構成 | READMEの一部サンプルやパスが現在の構成・API名と一致しない可能性がある（例: `Engine/` や `SingleDisplay`）。利用ガイドの記述を現行ヘッダーと照合して更新すると、導入時の迷いを減らせる。README自体もAI生成で正確性が低い旨を明記している。 |
| 低 | `DirectXGame/SHEngine` 全体 | 公開関数の引数で `std::string` や `std::vector` を値渡ししている箇所が散見される。コピーが不要な読み取り専用引数は `const&`、所有権を受け取る入力は値渡し＋moveなど、意図に応じた引数規約を揃えると不要コピーを減らしAPIの意図も読みやすくなる。 |
| 低 | `DirectXGame/SHEngine` 全体 | DirectX操作の失敗処理が `assert`、ログ、例外など複数の方式で扱われている。実行時エラーの報告方法と復旧方針を統一すると、Releaseビルドでの診断可能性と呼び出し側の扱いやすさが上がる。 |

## レビュー時の注意

- `const` はオブジェクト自身を変更しないことを示す。返却された非constポインタの先まで不変になるわけではないため、呼び出し側の期待に応じて返却型も検討する。
- 命名修正は公開ヘッダーの利用箇所とプロジェクト内の呼び出しをまとめて更新する。
- 設計パターンは導入自体を目的にせず、重複や変更頻度が確認できる範囲で適用する。
