#pragma once
#include <Utility/MyMath.h>
#include <Utility/DataStructures.h>
#include <string>
#include <unordered_map>

struct Node {
    std::string name;

    QuaternionTransform localTransform;
	Matrix4x4 localMatrix;

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


struct Primitive {
    uint32_t indexOffset;
    uint32_t indexCount = 3;
    uint32_t materialIndex;
};

struct Mesh {
    std::string name;

    std::vector<Vector3> position;
    std::vector<Vector3> normal;
	std::vector<Vector2> texcoord;
    std::vector<Vector4> color;
	std::vector<VertexInfluence> vertexInfluences;

    std::vector<uint32_t> indices;

    std::vector<Primitive> primitives;
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
	Matrix4x4 rootMatrix;
    std::vector<Joint> joints;
};

struct ModelData {
    std::vector<Node> nodes;
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
    Skeleton skeleton;
};

// Animation =======================================================================================

template <typename T>
struct Keyframe {
    float time;
    T value;
};
using KeyframeVector3 = Keyframe<Vector3>;
using KeyframeQuaternion = Keyframe<Quaternion>;

template<typename T>
struct AnimationCurve {
    std::vector<Keyframe<T>> keyframes;
};

struct NodeAnimation {
    AnimationCurve<Vector3> position;
    AnimationCurve<Quaternion> rotate;
    AnimationCurve<Vector3> scale;
};

struct Animation {
    float duration;
    std::unordered_map<std::string, NodeAnimation> nodeAnimations;
};
