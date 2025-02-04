/*
nifly
C++ NIF library for the Gamebryo/NetImmerse File Format
See the included GPLv3 LICENSE file
*/

#pragma once

#include "BasicTypes.hpp"

namespace nifly {
enum VertexAttribute : uint8_t {
	VA_POSITION = 0x0,
	VA_TEXCOORD0 = 0x1,
	VA_TEXCOORD1 = 0x2,
	VA_NORMAL = 0x3,
	VA_BINORMAL = 0x4,
	VA_COLOR = 0x5,
	VA_SKINNING = 0x6,
	VA_LANDDATA = 0x7,
	VA_EYEDATA = 0x8,
	VA_COUNT = 9
};

enum VertexFlags : uint16_t {
	VF_VERTEX = 1 << VA_POSITION,
	VF_UV = 1 << VA_TEXCOORD0,
	VF_UV_2 = 1 << VA_TEXCOORD1,
	VF_NORMAL = 1 << VA_NORMAL,
	VF_TANGENT = 1 << VA_BINORMAL,
	VF_COLORS = 1 << VA_COLOR,
	VF_SKINNED = 1 << VA_SKINNING,
	VF_LANDDATA = 1 << VA_LANDDATA,
	VF_EYEDATA = 1 << VA_EYEDATA,
	VF_FULLPREC = 0x400
};

const uint64_t DESC_MASK_VERT = 0xFFFFFFFFFFFFFFF0;
const uint64_t DESC_MASK_UVS = 0xFFFFFFFFFFFFFF0F;
const uint64_t DESC_MASK_NBT = 0xFFFFFFFFFFFFF0FF;
const uint64_t DESC_MASK_SKCOL = 0xFFFFFFFFFFFF0FFF;
const uint64_t DESC_MASK_DATA = 0xFFFFFFFFFFF0FFFF;
const uint64_t DESC_MASK_OFFSET = 0xFFFFFF0000000000;
const uint64_t DESC_MASK_FLAGS = ~(DESC_MASK_OFFSET);

class VertexDesc {
private:
	uint64_t desc = 0;

public:
	// Sets a specific flag
	void SetFlag(VertexFlags flag) { desc |= ((uint64_t) flag << 44); }

	// Removes a specific flag
	void RemoveFlag(VertexFlags flag) { desc &= ~((uint64_t) flag << 44); }

	// Checks for a specific flag
	bool HasFlag(VertexFlags flag) const { return ((desc >> 44) & flag) != 0; }

	// Sets the vertex size
	void SetSize(uint32_t size) {
		desc &= DESC_MASK_VERT;
		desc |= (uint64_t) size >> 2;
	}

	// Sets the dynamic vertex size
	void MakeDynamic() {
		desc &= DESC_MASK_UVS;
		desc |= 0x40;
	}

	// Return offset to a specific vertex attribute in the description
	uint32_t GetAttributeOffset(VertexAttribute attr) const { return (desc >> (4 * (uint8_t) attr + 2)) & 0x3C; }

	// Set offset to a specific vertex attribute in the description
	void SetAttributeOffset(VertexAttribute attr, uint32_t offset) {
		if (attr != VA_POSITION) {
			desc = ((uint64_t) offset << (4 * (uint8_t) attr + 2))
				   | (desc & ~(15 << (4 * (uint8_t) attr + 4)));
		}
	}

	void ClearAttributeOffsets() { desc &= DESC_MASK_OFFSET; }

	VertexFlags GetFlags() const { return VertexFlags((desc & DESC_MASK_OFFSET) >> 44); }

	void SetFlags(VertexFlags flags) { desc |= ((uint64_t) flags << 44) | (desc & DESC_MASK_FLAGS); }

	void Sync(NiStreamReversible& stream) { stream.Sync(desc); }
};

struct BSVertexData {
	// Single- or half-precision depending on IsFullPrecision() being true
	Vector3 vert;
	float bitangentX = 0.0f; // Maybe the dot product of the vert normal and the z-axis?

	Vector2 uv;

	uint8_t normal[3]{};
	uint8_t bitangentY = 0;
	uint8_t tangent[3]{};
	uint8_t bitangentZ = 0;

	uint8_t colorData[4]{};

	float weights[4]{};
	uint8_t weightBones[4]{};

	float eyeData = 0.0f;
};
} // namespace nifly
