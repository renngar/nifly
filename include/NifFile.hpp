/*
nifly
C++ NIF library for the Gamebryo/NetImmerse File Format
See the included GPLv3 LICENSE file
*/

#pragma once

#include "Factory.hpp"

#include <filesystem>

namespace nifly {
struct OptOptions {
	NiVersion targetVersion;
	bool headParts = false;
	bool removeParallax = true;
	bool calcBounds = true;
};

struct OptResult {
	bool versionMismatch = false;
	bool dupesRenamed = false;
	std::vector<std::string> shapesVColorsRemoved;
	std::vector<std::string> shapesNormalsRemoved;
	std::vector<std::string> shapesPartTriangulated;
	std::vector<std::string> shapesTangentsAdded;
	std::vector<std::string> shapesParallaxRemoved;
};

// Sort bone weights with indices
struct BoneWeightsSort {
	bool operator()(const SkinWeight& lhs, const SkinWeight& rhs) {
		if (lhs.weight == rhs.weight)
			return false;

		return rhs.weight < lhs.weight;
	}
};

struct NifLoadOptions {
	bool isTerrain = false;
};

struct NifSaveOptions {
	bool optimize = true;
	bool sortBlocks = true;
};

class NifFile {
private:
	NiHeader hdr;
	std::vector<std::unique_ptr<NiObject>> blocks;
	bool isValid = false;
	bool hasUnknown = false;
	bool isTerrain = false;

public:
	NifFile() = default;

	NifFile(const std::filesystem::path& fileName, const NifLoadOptions& options = NifLoadOptions()) {
		Load(fileName, options);
	}

	NifFile(std::istream& file, const NifLoadOptions& options = NifLoadOptions()) { Load(file, options); }

	NifFile(const NifFile& other) { CopyFrom(other); }

	NifFile& operator=(const NifFile& other) {
		CopyFrom(other);
		return *this;
	}

	NiHeader& GetHeader() { return hdr; }
	const NiHeader& GetHeader() const { return hdr; }
	void CopyFrom(const NifFile& other);

	int Load(const std::filesystem::path& fileName, const NifLoadOptions& options = NifLoadOptions());
	int Load(std::istream& file, const NifLoadOptions& options = NifLoadOptions());
	int Save(const std::filesystem::path& fileName, const NifSaveOptions& options = NifSaveOptions());
	int Save(std::ostream& file, const NifSaveOptions& options = NifSaveOptions());

	void Optimize();
	OptResult OptimizeFor(OptOptions& options);

	void PrepareData();
	void FinalizeData();

	bool IsValid() const { return isValid; }
	bool HasUnknown() const { return hasUnknown; }
	bool IsTerrain() const { return isTerrain; }
	bool IsSSECompatible() const;
	bool IsSSECompatible(NiShape* shape) const;

	void Create(const NiVersion& version);
	void Clear();

	// Link NiGeometryData to NiGeometry
	void LinkGeomData();
	void RemoveInvalidTris() const;

	static size_t GetVertexLimit();
	size_t GetTriangleLimit() const;

	NiNode* AddNode(const std::string& nodeName, const MatTransform& xformToParent, NiNode* parent = nullptr);
	void DeleteNode(const std::string& nodeName);
	static bool CanDeleteNode(NiNode* node);
	bool CanDeleteNode(const std::string& nodeName) const;
	std::string GetNodeName(const uint32_t blockID) const;
	void SetNodeName(const uint32_t blockID, const std::string& newName);

	uint32_t AssignExtraData(NiAVObject* target, std::unique_ptr<NiExtraData> extraData);

	// Explicitly sets the order of shapes to a new one.
	void SetShapeOrder(const std::vector<std::string>& order);
	static void SetSortIndex(const NiRef& ref, std::vector<uint32_t>& newIndices, uint32_t& newIndex);
	static void SetSortIndex(const NiRef* ref, std::vector<uint32_t>& newIndices, uint32_t& newIndex);
	void SortAVObject(NiAVObject* avobj, std::vector<uint32_t>& newIndices, uint32_t& newIndex);
	void SortShape(NiShape* shape, std::vector<uint32_t>& newIndices, uint32_t& newIndex);
	void SortGraph(NiNode* root, std::vector<uint32_t>& newIndices, uint32_t& newIndex);
	void PrettySortBlocks();

	template<class T = NiObject>
	uint32_t DeleteUnreferencedBlocks() {
		if (hasUnknown)
			return 0;

		uint32_t deletionCount = 0;
		hdr.DeleteUnreferencedBlocks<T>(GetBlockID(GetRootNode()), &deletionCount);
		return deletionCount;
	}

	bool DeleteUnreferencedNodes(int* deletionCount = nullptr);

	template<class T = NiObject>
	T* FindBlockByName(const std::string& name) const;
	uint32_t GetBlockID(NiObject* block) const;
	NiNode* GetParentNode(NiObject* block) const;
	void SetParentNode(NiObject* block, NiNode* parent);
	std::vector<NiNode*> GetNodes() const;

	NiShader* GetShader(NiShape* shape) const;
	NiMaterialProperty* GetMaterialProperty(NiShape* shape) const;
	NiStencilProperty* GetStencilProperty(NiShape* shape) const;

	uint32_t GetTextureSlot(NiShader* shader, std::string& outTexFile, const uint32_t texIndex = 0) const;
	void SetTextureSlot(NiShader* shader, std::string& inTexFile, const uint32_t texIndex = 0);
	void TrimTexturePaths();

	void CloneChildren(NiObject* block, NifFile* srcNif = nullptr);
	NiShape* CloneShape(NiShape* srcShape, const std::string& destShapeName, NifFile* srcNif = nullptr);
	uint32_t CloneNamedNode(const std::string& nodeName, NifFile* srcNif = nullptr);

	NiShape* CreateShapeFromData(const std::string& shapeName,
								 const std::vector<Vector3>* v,
								 const std::vector<Triangle>* t,
								 const std::vector<Vector2>* uv,
								 const std::vector<Vector3>* norms = nullptr);
	std::vector<std::string> GetShapeNames() const;
	std::vector<NiShape*> GetShapes() const;
	static bool RenameShape(NiShape* shape, const std::string& newName);
	bool RenameDuplicateShapes();
	void TriangulateShape(NiShape* shape);

	// GetChildren of a node ... templatized to allow any particular type to be queried.   useful for walking a node tree
	template<class T>
	std::vector<T*> GetChildren(NiNode* parent = nullptr, bool searchExtraData = false) const;

	NiNode* GetRootNode() const;
	void GetTree(std::vector<NiObject*>& result, NiObject* parent = nullptr) const;
	bool GetNodeTransformToParent(const std::string& nodeName, MatTransform& outTransform) const;
	// GetNodeTransform is deprecated.  Use GetNodeTransformToParent instead.
	bool GetNodeTransform(const std::string& nodeName, MatTransform& outTransform) const {
		return GetNodeTransformToParent(nodeName, outTransform);
	}
	// GetNodeTransformToGlobal calculates the transform from this node's
	// CS to the global CS by composing transforms up the node tree to the
	// root node.
	bool GetNodeTransformToGlobal(const std::string& nodeName, MatTransform& outTransform) const;
	// GetAbsoluteNodeTransform is deprecated.  Use GetNodeTransformToGlobal instead.
	bool GetAbsoluteNodeTransform(const std::string& nodeName, MatTransform& outTransform) const {
		return GetNodeTransformToGlobal(nodeName, outTransform);
	}
	bool SetNodeTransformToParent(const std::string& nodeName,
								  const MatTransform& inTransform,
								  const bool rootChildrenOnly = false);
	// SetNodeTransform is deprecated.  Use SetNodeTransformToParent instead.
	bool SetNodeTransform(const std::string& nodeName,
						  MatTransform& inTransform,
						  const bool rootChildrenOnly = false) {
		return SetNodeTransformToParent(nodeName, inTransform, rootChildrenOnly);
	}

	int GetShapeBoneList(NiShape* shape, std::vector<std::string>& outList) const;
	int GetShapeBoneIDList(NiShape* shape, std::vector<int>& outList) const;
	void SetShapeBoneIDList(NiShape* shape, std::vector<int>& inList);
	uint32_t GetShapeBoneWeights(NiShape* shape,
								 const uint32_t boneIndex,
								 std::unordered_map<uint16_t, float>& outWeights) const;

	// Looks up the shape's global-to-skin transform if it has it.
	// Otherwise, try to calculate it using skin-to-bone and node-to-global
	// transforms.  Returns false on failure.
	bool CalcShapeTransformGlobalToSkin(NiShape* shape, MatTransform& outTransforms) const;
	// Returns false if no such transform exists in the file, in which
	// case outTransform will not be changed.  Note that, even if this
	// function returns false, you can not assume that the global-to-skin
	// transform is the identity; it almost never is.
	bool GetShapeTransformGlobalToSkin(NiShape* shape, MatTransform& outTransform) const;
	// Does nothing if the shape has no such transform.
	void SetShapeTransformGlobalToSkin(NiShape* shape, const MatTransform& inTransform);
	bool GetShapeTransformSkinToBone(NiShape* shape,
									 const std::string& boneName,
									 MatTransform& outTransform) const;
	bool GetShapeTransformSkinToBone(NiShape* shape,
									 const uint32_t boneIndex,
									 MatTransform& outTransform) const;
	void SetShapeTransformSkinToBone(NiShape* shape,
									 const uint32_t boneIndex,
									 const MatTransform& inTransform);
	// Empty std::string for the bone name returns the overall skin transform for the shape.
	// GetShapeBoneTransform is deprecated.  Use GetShapeTransformGlobalToSkin
	// or GetShapeTransfromSkinToBone instead.
	bool GetShapeBoneTransform(NiShape* shape, const std::string& boneName, MatTransform& outTransform) const;
	// 0xFFFFFFFF on the bone index returns the overall skin transform for the shape.
	// GetShapeBoneTransform is deprecated.  Use GetShapeTransformGlobalToSkin
	// or GetShapeTransfromSkinToBone instead.
	bool GetShapeBoneTransform(NiShape* shape, const uint32_t boneIndex, MatTransform& outTransform) const;
	// 0xFFFFFFFF for the bone index sets the overall skin transform for the shape.
	// SetShapeBoneTransform is deprecated.  Use SetShapeTransformGlobalToSkin
	// or SetShapeTransfromSkinToBone instead.
	bool SetShapeBoneTransform(NiShape* shape, const uint32_t boneIndex, MatTransform& inTransform);
	bool SetShapeBoneBounds(const std::string& shapeName, const uint32_t boneIndex, BoundingSphere& inBounds);
	bool GetShapeBoneBounds(NiShape* shape, const uint32_t boneIndex, BoundingSphere& outBounds) const;
	void UpdateShapeBoneID(const std::string& shapeName, const uint32_t oldID, const uint32_t newID);
	void SetShapeBoneWeights(const std::string& shapeName,
							 const uint32_t boneIndex,
							 std::unordered_map<uint16_t, float>& inWeights);
	void SetShapeVertWeights(const std::string& shapeName,
							 const int vertIndex,
							 std::vector<uint8_t>& boneids,
							 std::vector<float>& weights) const;
	void ClearShapeVertWeights(const std::string& shapeName) const;

	static bool GetShapeSegments(NiShape* shape, NifSegmentationInfo& inf, std::vector<int>& triParts);
	static void SetShapeSegments(NiShape* shape,
								 const NifSegmentationInfo& inf,
								 const std::vector<int>& triParts);

	bool GetShapePartitions(NiShape* shape,
							std::vector<BSDismemberSkinInstance::PartitionInfo>& partitionInfo,
							std::vector<int>& triParts) const;
	void SetShapePartitions(NiShape* shape,
							const std::vector<BSDismemberSkinInstance::PartitionInfo>& partitionInfo,
							const std::vector<int>& triParts,
							const bool convertSkinInstance = true);
	void SetDefaultPartition(NiShape* shape);
	// DeletePartitions: partInds must be in sorted ascending order.
	void DeletePartitions(NiShape* shape, std::vector<uint32_t>& partInds);

	static bool ReorderTriangles(NiShape* shape, const std::vector<uint32_t>& triangleIndices);
	const std::vector<Vector3>* GetVertsForShape(NiShape* shape);
	const std::vector<Vector3>* GetNormalsForShape(NiShape* shape);
	const std::vector<Vector2>* GetUvsForShape(NiShape* shape);
	const std::vector<Color4>* GetColorsForShape(const std::string& shapeName);
	const std::vector<Vector3>* GetTangentsForShape(NiShape* shape);
	const std::vector<Vector3>* GetBitangentsForShape(NiShape* shape);
	const std::vector<float>* GetEyeDataForShape(NiShape* shape);
	bool GetVertsForShape(NiShape* shape, std::vector<Vector3>& outVerts) const;
	bool GetUvsForShape(NiShape* shape, std::vector<Vector2>& outUvs) const;
	bool GetColorsForShape(NiShape* shape, std::vector<Color4>& outColors) const;
	bool GetTangentsForShape(NiShape* shape, std::vector<Vector3>& outTang) const;
	bool GetBitangentsForShape(NiShape* shape, std::vector<Vector3>& outBitang) const;
	static bool GetEyeDataForShape(NiShape* shape, std::vector<float>& outEyeData);
	void SetVertsForShape(NiShape* shape, const std::vector<Vector3>& verts);
	void SetUvsForShape(NiShape* shape, const std::vector<Vector2>& uvs);
	void SetColorsForShape(const std::string& shapeName, const std::vector<Color4>& colors);
	void SetTangentsForShape(NiShape* shape, const std::vector<Vector3>& tangents);
	void SetBitangentsForShape(NiShape* shape, const std::vector<Vector3>& bitangents);
	static void SetEyeDataForShape(NiShape* shape, const std::vector<float>& eyeData);
	void InvertUVsForShape(NiShape* shape, bool invertX, bool invertY);
	void MirrorShape(NiShape* shape, bool mirrorX, bool mirrorY, bool mirrorZ);
	void SetNormalsForShape(NiShape* shape, const std::vector<Vector3>& norms);
	void CalcNormalsForShape(NiShape* shape,
							 const bool force = false,
							 const bool smooth = true,
							 const float smoothThresh = 60.0f);
	void CalcTangentsForShape(NiShape* shape);

	int ApplyNormalsFromFile(NifFile& srcNif, const std::string& shapeName);

	void GetRootTranslation(Vector3& outVec) const;

	void MoveVertex(NiShape* shape, const Vector3& pos, const int id);
	void OffsetShape(NiShape* shape,
					 const Vector3& offset,
					 std::unordered_map<uint16_t, float>* mask = nullptr);
	void ScaleShape(NiShape* shape, const Vector3& scale, std::unordered_map<uint16_t, float>* mask = nullptr);
	void RotateShape(NiShape* shape,
					 const Vector3& angle,
					 std::unordered_map<uint16_t, float>* mask = nullptr);

	NiAlphaProperty* GetAlphaProperty(NiShape* shape) const;
	uint32_t AssignAlphaProperty(
		NiShape* shape,
		std::unique_ptr<NiAlphaProperty> alphaProp); // uint16_t flags = 4844, uint16_t threshold = 128
	void RemoveAlphaProperty(NiShape* shape);

	void DeleteShape(NiShape* shape);
	void DeleteShader(NiShape* shape);
	void DeleteSkinning(NiShape* shape);
	void RemoveEmptyPartitions(NiShape* shape);
	bool DeleteVertsForShape(NiShape* shape, const std::vector<uint16_t>& indices);

	int CalcShapeDiff(NiShape* shape,
					  const std::vector<Vector3>* targetData,
					  std::unordered_map<uint16_t, Vector3>& outDiffData,
					  float scale = 1.0f);
	int CalcUVDiff(NiShape* shape,
				   const std::vector<Vector2>* targetData,
				   std::unordered_map<uint16_t, Vector3>& outDiffData,
				   float scale = 1.0f);

	void CreateSkinning(NiShape* shape);
	void SetShapeDynamic(const std::string& shapeName);

	// Maintains the number of and makeup of skin partitions, but updates the weighting values
	void UpdateSkinPartitions(NiShape* shape);
	// Update bone set flags
	void UpdatePartitionFlags(NiShape* shape);
};

template<class T>
std::vector<T*> NifFile::GetChildren(NiNode* parent, bool searchExtraData) const {
	std::vector<T*> result;
	T* n;

	if (parent == nullptr) {
		parent = GetRootNode();
		if (parent == nullptr)
			return result;
	}

	for (auto& child : parent->childRefs) {
		n = hdr.GetBlock<T>(child);
		if (n)
			result.push_back(n);
	}

	if (searchExtraData) {
		for (auto& extraData : parent->extraDataRefs) {
			n = hdr.GetBlock<T>(extraData);
			if (n)
				result.push_back(n);
		}
	}

	return result;
}
} // namespace nifly
