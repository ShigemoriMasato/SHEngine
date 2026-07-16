#pragma once
#include <Utility/MyMath.h>

struct Node {
    std::string name;

    QuaternionTransform localTransform;

    Matrix4x4 localMatrix;
    Matrix4x4 globalMatrix;

    int parent = -1;
    std::vector<uint32_t> children;

    int meshIndex = -1;
    int skinIndex = -1;
};

static constexpr int MAX_JOINTS_PER_VERTEX = 4;
struct VertexInfluence {
    uint32_t joint[MAX_JOINTS_PER_VERTEX];
    float weight[MAX_JOINTS_PER_VERTEX];
};

enum class VertexType : uint8_t {
    None            = 0,
    Position        = 1 << 0,
	Texcoord        = 1 << 1,
    Normal          = 1 << 2,
	Color           = 1 << 3,
	Influence       = 1 << 4
};

uint8_t operator|(VertexType a, VertexType b);
uint8_t operator|(uint8_t a, VertexType b);
uint8_t operator&(VertexType a, VertexType b);
uint8_t operator&(uint8_t a, VertexType b);
uint8_t operator~(VertexType a);

struct Primitive {
    uint32_t indexOffset;
    uint32_t indexCount = 3;
    uint32_t materialIndex;
};

struct Mesh {
    std::string name;

    std::vector<Vector4> position;
    std::vector<Vector3> normal;
	std::vector<Vector2> texcoord;
    std::vector<Vector4> color;
	std::vector<VertexInfluence> vertexInfluences;

    std::vector<uint32_t> indices;

    std::vector<Primitive> primitives;

	std::array<int, 32> drawDataIndices;
};

struct Material {
    std::string name;

    Vector4 baseColor = { 1,1,1,1 };

    float metallic = 1.0f;
    float roughness = 1.0f;

    int textureIndex = -1;
    int normalTexture = -1;
};

struct Joint {
    std::string name;
    uint32_t nodeIndex;
    Matrix4x4 inverseBindMatrix;
};

struct Skeleton {
    uint32_t rootNode;
    std::vector<Joint> joints;
};

struct Model {
    std::vector<Node> nodes;
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
    Skeleton skeleton;
};