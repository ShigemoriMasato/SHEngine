# SHEngineのコードレビュー
コミット済みコードを含むリポジトリの現状を対象に、GPUパーティクルの初期化と描画、通常／メッシュPSOの生成失敗処理、シェーダー再読み込みとキャッシュ管理、COMリソース寿命、セットアップスクリプト、削除・追加されたファイルの配置を確認した。スペル、境界アクセス、失敗後の処理継続、初期化データの整合性を重点的に調べた。

| 該当箇所 | 問題点 | 重要度 |
| --- | --- | --- |
| `DirectXGame/SHEngine/Core/DXDevice.cpp` `CompileShader()` | `DXC_OUT_ERRORS` が null の場合、成功後に `shaderError->Release()` を呼ぶため null dereference する可能性がある。成功時に診断Blobが必ず返る前提を置かず、null確認して解放する必要がある。 | 高 |
| `DirectXGame/SHEngine/Core/DXDevice.cpp` `CompileShader()` | コンパイラ診断文字列が警告だけの場合でも無条件に null を返すため、警告付きで成功したシェーダーまで失敗扱いになる。診断文字列の有無とコンパイル成否を分けて扱う必要がある。 | 中 |
| `DirectXGame/SHEngine/Render/PSO/PSOManager.cpp` `CreatePSO()` | シェーダーバイトコードが空の場合はnull PSOをキャッシュするが、後からシェーダーが修正されても同じConfigの `GetPSO()` はキャッシュ値を返すため、再コンパイル／再生成されない。 | 中 |
| `DirectXGame/SHEngine/Render/PSO/PSOEditor.cpp` `SetPSO()` | `nowConfig_` と同じ設定ならPSOマネージャーを呼ばず成功扱いする。PSOを外部からResetした後も `nowConfig_` は維持されるため、同じ設定で呼び出すとPSOが再生成・再設定されず、コマンドリストに無効なPSO状態が残る可能性がある。Reset時にエディタの設定キャッシュも無効化する必要がある。 | 高 |
| `DirectXGame/SHEngine/Render/PSO/PSOManagerForMS.cpp` `SetPSO()` | ASが空の場合はASバイトコード取得を省略するが、`D3D12_SHADER_BYTECODE as` をゼロ初期化していないままPSOストリームに渡す。AS省略時に未初期化ポインター／長さを含むPSO記述子となる。 | 高 |
| `DirectXGame/GameEngine/Particle/System/ParticleProcessor.cpp`、`ParticleSystem.cpp` | `Initialize()` は初期化Computeを記録するだけで、ParticleSceneの初回描画前にComputeコマンドを実行する処理が確認できない。さらに `ParticleSystem::Update()` はレンダラー更新のみで、AllocateやEmitter処理も未実装。フリ―リストや粒子データがGPU上で初期化されず、粒子生成も行われない。 | 高 |
| `DirectXGame/GameEngine/Particle/System/ParticleData.cpp`、`Assets/Shader/Compute/Particle/System/Initialize.CS.hlsl` | 初期化シェーダーは位置を未使用粒子用sentinelに設定した直後、`index * 0.3f` で上書きする。Mesh shader側のsentinel判定が成立しないため、未割り当て粒子も描画対象になる。 | 高 |
| `DirectXGame/Assets/Shader/Mesh/Particle/Quad.MS.hlsl` | `parentIndices[threadID]` を使って `parentMatrices` を参照する際、親行列バッファの有効要素数を検証していない。無効な親IDが設定されるとGPU上で範囲外読み出しになる。 | 中 |
| `setup.bat` `:BUILD_ASSIMP`、`:BUILD_FREETYPE` | 呼び出している `:CLEAN_ASSIMP_FILES` と `:CLEAN_FREETYPE_FILES` ラベルが定義されていない。共通の `:END` が `CLEAN_GENERATED_FILES` で各ライブラリのclone／生成プロジェクトを削除するため、単体ビルド後にヘッダーコピーが参照するディレクトリも失われる。 | 高 |
| `DirectXGame/SHEngine/Core/DXDevice.cpp` | コンパイル成功ログの `Compile Successd` は `Compile Succeeded` のスペルミス。 | 低 |
| `DirectXGame/GameEngine/Particle/Emitter.cpp` と `GameEngine/Particle/System/` | Emitterだけが `Particle` 直下で、関連するData／Processor／Rendererは `System` 配下に置かれている。同一機能の構成要素が分散し、構成を追いにくい。 | 低 |
| `DirectXGame/AppSource/Game/`、`AppSource/Sand/`、`AppSource/Title/` | ゲーム、テトリス、砂シミュレーター、タイトルシーンなど多数のソースが削除され、起動シーンもParticleSceneに切り替わっている。意図的な機能整理か判断できる説明が差分にないため、削除・移行方針が分かる記録が必要。 | 高 |

## 実行した検査項目
- スペルチェック（英語ログ、識別子、コメント）
- 想定外エラー（COM失敗処理、DXC診断Blob、PSO失敗後の描画、未初期化PSO記述子）
- 粒子バッファの初期化、Compute実行、未使用粒子判定、親行列参照
- PSO／シェーダーキャッシュの失敗・リセット後の挙動
- ファイル階層、プロジェクト登録、削除ファイルとセットアップスクリプトの整合性