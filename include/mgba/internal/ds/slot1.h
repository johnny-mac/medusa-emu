/* Copyright (c) 2013-2017 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef DS_SLOT1_H
#define DS_SLOT1_H

#include <mgba-util/common.h>

CXX_GUARD_START

#include <mgba/core/log.h>
#include <mgba/core/timing.h>

mLOG_DECLARE_CATEGORY(DS_SLOT1);

DECL_BITFIELD(DSSlot1AUXSPICNT, uint16_t);
DECL_BITS(DSSlot1AUXSPICNT, Baud, 0, 2);
DECL_BIT(DSSlot1AUXSPICNT, CSHold, 6);
DECL_BIT(DSSlot1AUXSPICNT, Busy, 7);
DECL_BIT(DSSlot1AUXSPICNT, SPIMode, 13);
DECL_BIT(DSSlot1AUXSPICNT, DoIRQ, 14);
DECL_BIT(DSSlot1AUXSPICNT, Enable, 15);

DECL_BITFIELD(DSSlot1ROMCNT, uint32_t);
DECL_BITS(DSSlot1ROMCNT, Delay, 0, 12);
DECL_BITS(DSSlot1ROMCNT, Gap, 16, 5);
DECL_BIT(DSSlot1ROMCNT, WordReady, 23);
DECL_BITS(DSSlot1ROMCNT, BlockSize, 24, 3);
DECL_BIT(DSSlot1ROMCNT, TransferRate, 27);
DECL_BIT(DSSlot1ROMCNT, BlockBusy, 31);

enum DSSavedataType {
	DS_SAVEDATA_AUTODETECT = -1,
	DS_SAVEDATA_FORCE_NONE = 0,
	DS_SAVEDATA_EEPROM512 = 1,
	DS_SAVEDATA_EEPROM = 2,
	DS_SAVEDATA_FLASH = 3
};

struct VFile;
struct DSSlot1 {
	uint8_t command[8];
	uint32_t address;
	uint32_t transferSize;
	uint32_t transferRemaining;
	struct mTimingEvent transferEvent;
	uint8_t readBuffer[4];

	int dmaSource;

	enum DSSavedataType savedataType;
	bool hasIR;
	struct mTimingEvent spiEvent;
	bool spiHoldEnabled;
	uint8_t spiCommand;
	uint8_t statusReg;
	int spiAddressingRemaining;
	uint32_t spiAddress;
	int spiAddressingBits;
	uint32_t spiSize;

	uint8_t* spiData;
	struct VFile* spiVf;
	struct VFile* spiRealVf;
};

struct DS;
struct DSCommon;
void DSSlot1SPIInit(struct DS* ds, struct VFile* vf);
void DSSlot1Reset(struct DS* ds);

DSSlot1AUXSPICNT DSSlot1Configure(struct DS* ds, DSSlot1AUXSPICNT config);
DSSlot1ROMCNT DSSlot1Control(struct DS* ds, DSSlot1ROMCNT control);
void DSSlot1WriteSPI(struct DSCommon* dscore, uint8_t datum);
void DSSlot1ConfigureSPI(struct DS* ds, uint32_t paramPtr);

struct GBADMA;
void DSSlot1ScheduleDMA(struct DSCommon* dscore, int number, struct GBADMA* info);

uint32_t DSSlot1Read(struct DS* ds);

CXX_GUARD_END

#endif
