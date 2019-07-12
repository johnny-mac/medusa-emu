/* Copyright (c) 2013-2017 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef DS_GX_H
#define DS_GX_H

#include <mgba-util/common.h>

CXX_GUARD_START

#include <mgba/core/interface.h>
#include <mgba/core/log.h>
#include <mgba/core/timing.h>
#include <mgba/internal/ds/matrix.h>
#include <mgba-util/circle-buffer.h>

mLOG_DECLARE_CATEGORY(DS_GX);

#define DS_GX_POLYGON_BUFFER_SIZE 2048
#define DS_GX_VERTEX_BUFFER_SIZE 6144

DECL_BITFIELD(DSRegGXSTAT, uint32_t);
DECL_BIT(DSRegGXSTAT, TestBusy, 0);
DECL_BIT(DSRegGXSTAT, BoxTestResult, 1);
DECL_BITS(DSRegGXSTAT, PVMatrixStackLevel, 8, 5);
DECL_BIT(DSRegGXSTAT, ProjMatrixStackLevel, 13);
DECL_BIT(DSRegGXSTAT, MatrixStackBusy, 14);
DECL_BIT(DSRegGXSTAT, MatrixStackError, 15);
DECL_BITS(DSRegGXSTAT, FIFOEntries, 16, 9);
DECL_BIT(DSRegGXSTAT, FIFOFull, 24);
DECL_BIT(DSRegGXSTAT, FIFOLtHalf, 25);
DECL_BIT(DSRegGXSTAT, FIFOEmpty, 26);
DECL_BIT(DSRegGXSTAT, Busy, 27);
DECL_BITS(DSRegGXSTAT, DoIRQ, 30, 2);

DECL_BITFIELD(DSGXTexParams, uint32_t);
DECL_BITS(DSGXTexParams, VRAMBase, 0, 16);
DECL_BIT(DSGXTexParams, SRepeat, 16);
DECL_BIT(DSGXTexParams, TRepeat, 17);
DECL_BIT(DSGXTexParams, SMirror, 18);
DECL_BIT(DSGXTexParams, TMirror, 19);
DECL_BITS(DSGXTexParams, SSize, 20, 3);
DECL_BITS(DSGXTexParams, TSize, 23, 3);
DECL_BITS(DSGXTexParams, Format, 26, 3);
DECL_BIT(DSGXTexParams, 0Transparent, 29);
DECL_BITS(DSGXTexParams, CoordTfMode, 30, 2);

DECL_BITFIELD(DSGXPolygonAttrs, uint32_t);
DECL_BITS(DSGXPolygonAttrs, Lights, 0, 4);
DECL_BITS(DSGXPolygonAttrs, Mode, 4, 2);
DECL_BIT(DSGXPolygonAttrs, BackFace, 6);
DECL_BIT(DSGXPolygonAttrs, FrontFace, 7);
DECL_BIT(DSGXPolygonAttrs, UpdateDepth, 11);
// TODO
DECL_BITS(DSGXPolygonAttrs, Alpha, 16, 5);
DECL_BITS(DSGXPolygonAttrs, Id, 24, 6);

enum DSGXCommand {
	DS_GX_CMD_NOP = 0,
	DS_GX_CMD_MTX_MODE = 0x10,
	DS_GX_CMD_MTX_PUSH = 0x11,
	DS_GX_CMD_MTX_POP = 0x12,
	DS_GX_CMD_MTX_STORE = 0x13,
	DS_GX_CMD_MTX_RESTORE = 0x14,
	DS_GX_CMD_MTX_IDENTITY = 0x15,
	DS_GX_CMD_MTX_LOAD_4x4 = 0x16,
	DS_GX_CMD_MTX_LOAD_4x3 = 0x17,
	DS_GX_CMD_MTX_MULT_4x4 = 0x18,
	DS_GX_CMD_MTX_MULT_4x3 = 0x19,
	DS_GX_CMD_MTX_MULT_3x3 = 0x1A,
	DS_GX_CMD_MTX_SCALE = 0x1B,
	DS_GX_CMD_MTX_TRANS = 0x1C,
	DS_GX_CMD_COLOR = 0x20,
	DS_GX_CMD_NORMAL = 0x21,
	DS_GX_CMD_TEXCOORD = 0x22,
	DS_GX_CMD_VTX_16 = 0x23,
	DS_GX_CMD_VTX_10 = 0x24,
	DS_GX_CMD_VTX_XY = 0x25,
	DS_GX_CMD_VTX_XZ = 0x26,
	DS_GX_CMD_VTX_YZ = 0x27,
	DS_GX_CMD_VTX_DIFF = 0x28,
	DS_GX_CMD_POLYGON_ATTR = 0x29,
	DS_GX_CMD_TEXIMAGE_PARAM = 0x2A,
	DS_GX_CMD_PLTT_BASE = 0x2B,
	DS_GX_CMD_DIF_AMB = 0x30,
	DS_GX_CMD_SPE_EMI = 0x31,
	DS_GX_CMD_LIGHT_VECTOR = 0x32,
	DS_GX_CMD_LIGHT_COLOR = 0x33,
	DS_GX_CMD_SHININESS = 0x34,
	DS_GX_CMD_BEGIN_VTXS = 0x40,
	DS_GX_CMD_END_VTXS = 0x41,
	DS_GX_CMD_SWAP_BUFFERS = 0x50,
	DS_GX_CMD_VIEWPORT = 0x60,
	DS_GX_CMD_BOX_TEST = 0x70,
	DS_GX_CMD_POS_TEST = 0x71,
	DS_GX_CMD_VEC_TEST = 0x72,

	DS_GX_CMD_MAX
};

#pragma pack(push, 1)
struct DSGXEntry {
	uint8_t command;
	uint8_t params[4];
};
#pragma pack(pop)

struct DSGXVertex {
	// World coords
	int16_t coord[3]; // 4.12

	// Color/Texcoords
	uint16_t color; // 5.5.5
	int16_t st[2]; // 12.4

	// Viewport coords
	int32_t viewCoord[4];
	int16_t vs; // 12.4
	int16_t vt; // 12.4

	int viewportX;
	int viewportY;
	int viewportWidth;
	int viewportHeight;
};

struct DSGXPolygon {
	DSGXPolygonAttrs polyParams;
	DSGXTexParams texParams;
	uint32_t palBase;
	int verts;
	unsigned vertIds[10];
};

struct DSGXRenderer {
	void (*init)(struct DSGXRenderer* renderer);
	void (*reset)(struct DSGXRenderer* renderer);
	void (*deinit)(struct DSGXRenderer* renderer);

	void (*invalidateTex)(struct DSGXRenderer* renderer, int slot);
	void (*setRAM)(struct DSGXRenderer* renderer, struct DSGXVertex* verts, struct DSGXPolygon* polys, unsigned polyCount, bool wSort);
	void (*drawScanline)(struct DSGXRenderer* renderer, int y);
	void (*getScanline)(struct DSGXRenderer* renderer, int y, const color_t** output);
	void (*writeRegister)(struct DSGXRenderer* renderer, uint32_t address, uint16_t value);

	uint16_t* tex[4];
	uint16_t* texPal[6];

	uint16_t* toonTable;
};

struct DSGXLight {
	int16_t color;
	int16_t x;
	int16_t y;
	int16_t z;
};

struct DS;
struct DSGX {
	struct DS* p;
	struct DSGXRenderer* renderer;
	struct CircleBuffer fifo;
	struct CircleBuffer pipe;

	struct mTimingEvent fifoEvent;
	int dmaSource;

	int outstandingParams[4];
	uint8_t outstandingCommand[4];
	struct DSGXEntry outstandingEntry;

	int activeParams;
	struct DSGXEntry activeEntries[32];

	bool swapBuffers;
	bool wSort;
	int bufferIndex;
	int vertexIndex;
	int pendingVertexIndex;
	int polygonIndex;
	struct DSGXVertex* vertexBuffer[2];
	struct DSGXPolygon* polygonBuffer[2];
	struct DSGXVertex pendingVertices[8];
	int pendingVertexIds[8];
	bool reverseWinding;

	uint16_t* tex[4];
	uint16_t* texPal[6];

	int mtxMode;
	int pvMatrixPointer;
	struct DSGXMatrix projMatrixStack;
	struct DSGXMatrix texMatrixStack;
	struct DSGXMatrix posMatrixStack[32];
	struct DSGXMatrix vecMatrixStack[32];

	struct DSGXMatrix projMatrix;
	struct DSGXMatrix texMatrix;
	struct DSGXMatrix posMatrix;
	struct DSGXMatrix vecMatrix;

	struct DSGXMatrix clipMatrix;

	struct DSGXLight lights[4];
	int16_t diffuse;
	int16_t ambient;
	int16_t specular;
	int16_t emit;

	int viewportX1;
	int viewportY1;
	int viewportX2;
	int viewportY2;
	int viewportWidth;
	int viewportHeight;

	int vertexMode;
	struct DSGXVertex currentVertex;
	struct DSGXPolygon nextPoly;
	struct DSGXPolygon currentPoly;
};

void DSGXInit(struct DSGX*);
void DSGXDeinit(struct DSGX*);
void DSGXReset(struct DSGX*);
void DSGXAssociateRenderer(struct DSGX* video, struct DSGXRenderer* renderer);

uint16_t DSGXWriteRegister(struct DSGX*, uint32_t address, uint16_t value);
uint32_t DSGXWriteRegister32(struct DSGX*, uint32_t address, uint32_t value);

void DSGXFlush(struct DSGX*);
void DSGXUpdateGXSTAT(struct DSGX*);

struct GBADMA;
struct DSCommon;
void DSGXScheduleDMA(struct DSCommon* dscore, int number, struct GBADMA* info);

CXX_GUARD_END

#endif
