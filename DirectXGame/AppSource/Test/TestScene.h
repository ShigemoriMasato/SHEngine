#pragma once
#include <Scene/IScene.h>
#include <Camera/DebugCamera.h>
#include <Tool/Grid/Grid.h>

#include <Camera/Editor/CameraEditor.h>
#include <Tool/DecoEditor/DecoEditor.h>
#include <Game/Effect/FallPol/FallPolygonEmitter.h>

#include <Tool/ModelDrawer/ModelDrawer.h>

class TestScene : public IScene {
public:

	~TestScene();

	void Initialize() override;
	std::unique_ptr<IScene> Update() override;
	void Draw() override;

private:

	void Save();
	void Load();

	void SelectFile();

	FallPolygonEmitter::MeshList CreateMeshList();
	void DecomposeMeshList(const FallPolygonEmitter::MeshList& meshList);

	std::unique_ptr<DebugCamera> debugCamera_;
	std::unique_ptr<Camera> gameCamera_;
	Camera orthoCamera_;
	std::unique_ptr<Grid> grid_;

	CameraEditor cameraEditor_;
	std::unique_ptr<DecoEditor> decoEditor_;

	CameraCurveData cameraCurveData_;
	DecoObjData decoObjData_;
	std::unordered_map<std::string, std::map<int, std::pair<Vector4, uint32_t>>> decoObjDataBuffer_;

	std::unique_ptr<SHEngine::Screen::Display> gameDisplay_;

	ModelDrawer cameraRenderer_{};

private:

	char currentFileName_[256] = "";
	const std::string basePath_ = "Game/StageConfig/";
	const std::string extension_ = ".bin";

	std::vector<std::string> fileList_ = {};

	
};
