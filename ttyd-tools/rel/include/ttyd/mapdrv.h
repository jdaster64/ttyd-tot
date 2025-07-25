#pragma once

#include <gc/types.h>

#include <cstdint>
#include <cstddef>

namespace ttyd::mapdrv {

extern "C" {

/*

struct DisplayList {
    u32 offset;      // 0x00 - Offset to vertex data
    u32 vertexCount; // 0x04 - Number of vertices
};

 struct PackedDisplayList {
    u32 offset;      // 0x00 - Offset to packed data
    u16 vertexCount; // 0x04 - Number of vertices
    u8 padding[2];   // 0x06
};

*/

struct MapFileMesh {
	uint8_t unk0; //0x0
	uint8_t padding[2]; //0x1
	uint8_t isPackedDisplay; //0x3, BOOL
	int32_t displayListCount; //0x4
	uint32_t elementMask; //0x8
	uint32_t vcdTableOffset; //0xC
    union {
        uint32_t* displayLists;  // 0x10
        uint8_t* packedDisplayLists;  // 0x10
    } displayLists[];
};

struct MapFileJointPart {
	void* material; //0x0
	MapFileMesh* mesh; //0x4
};

struct MapJointDrawMode {
	uint8_t unk0[0x1 - 0x0]; //0x0
	uint8_t cullMode; //0x1
	uint8_t unk2[0x8 - 0x2]; //0x2
	int32_t unk8; //0x8
	uint8_t unkC[0x10 - 0xC]; //0xC
};

struct MapFileJoint {
	const char* name; //0x0
	const char* type; //0x4
	struct MapFileJoint* parent; //0x8
	struct MapFileJoint* child; //0xC
	struct MapFileJoint* next; //0x10
	struct MapFileJoint* prev; //0x14
	gc::vec3 scale; //0x18
	gc::vec3 rotation; //0x24
	gc::vec3 translation; //0x30
	gc::vec3 bboxMin; //0x3C
	gc::vec3 bboxMax; //0x48
	uint8_t field_0x54[0x58 - 0x54]; //0x54
	MapJointDrawMode* drawMode; //0x58
	int32_t partCount; //0x5C
	MapFileJointPart parts[]; //0x60+
};

typedef struct MapFileHeader {
	int32_t fileSize; //0x0
	int32_t dataSize; //0x4
	int32_t relCount; //0x8, TODO: rename?
	int32_t chunkCount; //0xC, TODO: rename?
	int32_t unused; //0x10
	uint8_t reserved[0xC]; //0x14
} MapFileHeader;

//entry in the chunk table
typedef struct MapFileChunk {
	uint32_t offset; //0x0, relative offset into data
	uint32_t string; //0x4, relative offset into strings
} MapFileChunk;

typedef struct MapFileInfo {
	const char* version; //0x0
	MapFileJoint* joint; //0x4
	const char* mapName; //0x8
	const char* hitName; //0xC
} MapFileInfo;

typedef struct MapHeader {
	MapFileHeader file; //0x0
	void* data; //0x20, TODO re-type?
	void* rel; //0x24, TODO: re-type?
	MapFileChunk* chunks; //0x28
	void* unused; //0x2C
	const char* strings; //0x30
	uint8_t field_0x34[0x3C - 0x34]; //0x34
	int32_t field_0x3C; //0x3C, dispFlags something?
	uint8_t field_0x40[0x44 - 0x40]; //0x40
} MapHeader;

typedef struct MapObject {
	uint8_t field_0x0[0x8 - 0x0]; //0x0
	MapFileJoint* joints; //0x8
	uint8_t field_0xC[0x1C - 0xC]; //0xC
	gc::mtx34 modelWorldMtx; //0x1C
	uint8_t field_0x4C[0xAC - 0x4C]; //0x4C
	gc::mtx34 unkAC; //0xAC
	uint8_t field_0xDC[0xE0 - 0xDC]; //0xDC
	struct MapObject* parent; //0xE0
	struct MapObject* child; //0xE4
	struct MapObject* next; //0xE8
	uint8_t field_0xEC[0x134 - 0xEC]; //0xEC
} MapObject;

typedef struct MapError {
	char field_0x0[32]; //0x0
	int32_t field_0x20; //0x20
} MapError;

typedef struct MapEntryAnimData {
	uint8_t field_0x0[0x20 - 0x0]; //0x0
} MapEntryAnimData;

typedef struct MapEntry {
	int32_t count; //0x0
	uint16_t flags; //0x4
	char name[16]; //0x6
	uint8_t field_0x16[0x38 - 0x16]; //0x16
	MapHeader header; //0x38
	void* dat; //0x7C, ./map/%s/map.dat
	uint32_t datSize; //0x80
	void* tpl; //0x84, ./map/%s/texture.tpl
	uint32_t tplSize; //0x88
	uint8_t field_0x8C[0xA8 - 0x8C]; //0x8C
	MapObject* rootMapObj; //0xA8
	struct HitObj* rootHitObj; //0xAC
	uint8_t field_0xB0[0x150 - 0xB0]; //0xB0
	int32_t numJoints; //0x150, TODO: rename? see: hitNumJoints
	MapObject* objects; //0x154, TODO: rename? see: hitNumJoints
	int32_t hitNumJoints; //0x158
	struct HitObj* hitObjects; //0x15C
	uint8_t field_0x160[0x164 - 0x160]; //0x160
	MapEntryAnimData* animData; //0x164
	uint8_t field_0x168[0x178 - 0x168]; //0x168
} MapEntry;

typedef struct MapWork {
	MapEntry entries[2]; //0x0
	int32_t unk2F0; //0x2F0
} MapWork;

// mapObjGetFlushColor
// mapGrpSetFlushColor
// mapObjSetFlushColor
// mapGrpFlushOff
// mapGrpFlushOn
// mapObjFlushOff
// mapObjFlushOn
// mapSetTevCallback
// mapTestXLU
// spline_getvalue
// spline_maketable
// mapSetProjgc::mtx34
// mapSetProjTexObj
// mapGrpSetColor
// setColor
// mapObjSetColor
// mapBlendOff2
// mapBlendOff
// mapGetBlend2
// mapGetBlend
// mapSetBlend2
// mapSetBlend
// mapFogOff
// mapFogOn
// mapGetFog
// mapSetFog
// mapObjGetPos
// mapObjGetPosSub
// mapObjScale
// mapObjTranslate
// mapObjRotate
// mapGrpClearOffScreen
// mapObjClearOffScreen
// mapGrpSetOffScreen
// mapObjSetOffScreen
// _setOffScrnId
// mapGrpFlagOff
// mapGrpFlagOn
// mapFlgOff
// mapFlgOn
// mapObjFlagOff
// mapObjFlagOn
// mapGetMapObj
// mapSearchDmdJoint
// mapSearchDmdJointSub2
// mapSearchDmdJointSub
// mapSetPolygonVtxDesc
// mapSetPolygon
// mapSetMaterialLight
// mapResetPaperAmbColor
// mapSetPaperAmbColor
// mapSetMaterial
// mapSetTextureMatrix
// mapSetMaterialFog
// mapSetMaterialLastStageBlend
// mapSetMaterialTev
// mapSetLight
// mapDisp
// mapDispMapGrp_bbox
// mapDispMapObj_bbox
// _mapDispMapGrp_NoMaterial
// _mapDispMapObj_NoMaterial
// test_kururing_mapdisp
// mapDispMapGrp_off
// mapDispMapObj_off
// mapDispMapGrp
// mapDispMapObj
// _mapDispMapGrp
// _mapDispMapObj
// mapMain
// mapReCalcMatrix
// mapCalcAnimMatrix
// bmapLoad
// bmapUnLoad
// mapLoad
// mapPreLoad
// mapUnLoad
// _mapLoad
// mapSetPlayRate
// mapReStartAnimationAll
// mapPauseAnimationAll
// mapReStartAnimation
// mapPauseAnimation
// mapPlayAnimationLv
// mapCheckAnimation
// mapSearchAnmObj
// makeDisplayList
// mapEntry
// mapEntrySub
// _mapEnt
// mapBuildTexture
// mapInit
// mapGetBoundingBox
// mapGetJoints
// mapGetJointsSub
// mapGetActiveGroup
// mapGetWork
// N_mapDispOn
// N_mapDispOff
// getMapDataDvdRoot

}

}