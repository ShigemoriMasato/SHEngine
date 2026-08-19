#pragma once
#include <Game/Effect/FallPol/FallPolygonEmitter.h>

class FallEmitterEditor {
public:

	void Initialize();
	void Update();
	void Draw();

	void SetMeshList(const FallPolygonEmitter::MeshList& meshList) { meshList_ = meshList; }
	const FallPolygonEmitter::MeshList& GetMeshList() const { return meshList_; }

private:

	FallPolygonEmitter::MeshList meshList_;

};
