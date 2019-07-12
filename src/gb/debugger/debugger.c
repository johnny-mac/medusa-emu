/* Copyright (c) 2013-2019 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
 #include <mgba/internal/gb/debugger/debugger.h>

 #include <mgba/core/core.h>
 #include <mgba/internal/debugger/cli-debugger.h>
 #include <mgba/internal/gb/gb.h>
 #include <mgba/internal/gb/io.h>
 #include <mgba/internal/gb/memory.h>
 #include <mgba/internal/lr35902/debugger/debugger.h>

static const struct LR35902Segment _GBSegments[] = {
	{ .name = "ROM", .start = GB_BASE_CART_BANK1, .end = GB_BASE_VRAM },
	{ .name = "RAM", .start = GB_BASE_EXTERNAL_RAM, .end = GB_BASE_WORKING_RAM_BANK0 },
	{ 0 }
};

static const struct LR35902Segment _GBCSegments[] = {
	{ .name = "ROM", .start = GB_BASE_CART_BANK1, .end = GB_BASE_VRAM },
	{ .name = "RAM", .start = GB_BASE_EXTERNAL_RAM, .end = GB_BASE_WORKING_RAM_BANK0 },
	{ .name = "WRAM", .start = GB_BASE_WORKING_RAM_BANK1, .end = 0xE000 },
	{ .name = "VRAM", .start = GB_BASE_VRAM, .end = GB_BASE_EXTERNAL_RAM },
	{ 0 }
};

static void _printStatus(struct CLIDebuggerSystem* debugger) {
	struct CLIDebuggerBackend* be = debugger->p->backend;
	struct GB* gb = debugger->p->d.core->board;
	be->printf(be, "IE: %02X  IF: %02X  IME: %i\n", gb->memory.ie, gb->memory.io[REG_IF], gb->memory.ime);
	be->printf(be, "LCDC: %02X  STAT: %02X  LY: %02X\n", gb->memory.io[REG_LCDC], gb->memory.io[REG_STAT] | 0x80, gb->memory.io[REG_LY]);
	be->printf(be, "Next video mode: %i\n", mTimingUntil(&gb->timing, &gb->video.modeEvent) / 4);
}

struct mDebuggerPlatform* GBDebuggerCreate(struct GB* gb) {
	struct LR35902Debugger* platform = (struct LR35902Debugger*) LR35902DebuggerPlatformCreate();
	if (gb->model >= GB_MODEL_CGB) {
		platform->segments = _GBCSegments;
	} else {
		platform->segments = _GBSegments;
	}
	platform->printStatus = _printStatus;
	return &platform->d;
}