/* Copyright (c) 2013-2017 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef DS_GX_SOFTWARE_H
#define DS_GX_SOFTWARE_H

#include <mgba-util/common.h>

CXX_GUARD_START

#include <mgba/internal/ds/gx.h>
#include <mgba/internal/ds/video.h>
#include <mgba-util/table.h>
#include <mgba-util/vector.h>

struct DSGXSoftwarePolygon {
	struct DSGXPolygon* poly;
	unsigned polyId;
	DSGXPolygonAttrs polyParams;
	DSGXTexParams texParams;
	uint16_t* texBase;
	uint16_t* palBase;
	int texFormat;
	int blendFormat;
	int texW;
	int texH;
	int minY;
	int maxY;
};

struct DSGXSoftwareEdge {
	unsigned polyId;
	int32_t y0; // 20.12
	int32_t x0; // 20.12
	int32_t z0; // 20.12
	int32_t w0; // 20.12
	int64_t wr0;
	uint8_t cr0;
	uint8_t cg0;
	uint8_t cb0;
	int16_t s0;
	int16_t t0;

	int32_t y1; // 20.12
	int32_t x1; // 20.12
	int32_t z1; // 20.12
	int32_t w1; // 20.12
	int64_t wr1;
	uint8_t cr1;
	uint8_t cg1;
	uint8_t cb1;
	int16_t s1;
	int16_t t1;
};

struct DSGXSoftwareEndpoint {
	int32_t coord[4]; // 20.12
	int64_t wRecip;
	uint8_t cr;
	uint8_t cg;
	uint8_t cb;
	int16_t s;
	int16_t t;

	int64_t stepZ;
	int64_t stepW;
	int64_t stepR;
	int64_t stepG;
	int64_t stepB;
	int64_t stepS;
	int64_t stepT;
};

struct DSGXSoftwareSpan {
	struct DSGXSoftwarePolygon* poly;
	int polyId;
	struct DSGXSoftwareEndpoint ep[2];
	struct DSGXSoftwareEndpoint step;
};

DECLARE_VECTOR(DSGXSoftwarePolygonList, struct DSGXSoftwarePolygon);
DECLARE_VECTOR(DSGXSoftwareEdgeList, struct DSGXSoftwareEdge);

struct DSGXSoftwareRenderer {
	struct DSGXRenderer d;

	struct DSGXSoftwarePolygonList activePolys;
	struct DSGXSoftwareEdgeList activeEdges;

	int32_t* depthBuffer;
	uint16_t* stencilBuffer;
	color_t* scanlineCache;
	int sort;
	uint16_t clearStencil;
	color_t clearColor;
	uint32_t clearDepth;
	bool flushPending;

	struct DSGXVertex* verts;
};

void DSGXSoftwareRendererCreate(struct DSGXSoftwareRenderer* renderer);

CXX_GUARD_END

#endif
