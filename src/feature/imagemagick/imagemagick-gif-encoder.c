/* Copyright (c) 2013-2015 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "imagemagick-gif-encoder.h"

#include <mgba/internal/gba/gba.h>
#include <mgba/gba/interface.h>
#include <mgba-util/string.h>

static void _magickPostVideoFrame(struct mAVStream*, const color_t* pixels, size_t stride);
static void _magickVideoDimensionsChanged(struct mAVStream*, unsigned width, unsigned height);
static void _magickVideoFrameRateChanged(struct mAVStream*, unsigned numerator, unsigned denominator);

void ImageMagickGIFEncoderInit(struct ImageMagickGIFEncoder* encoder) {
	encoder->wand = 0;

	encoder->d.videoDimensionsChanged = _magickVideoDimensionsChanged;
	encoder->d.postVideoFrame = _magickPostVideoFrame;
	encoder->d.postAudioFrame = 0;
	encoder->d.postAudioBuffer = 0;
	encoder->d.videoFrameRateChanged = _magickVideoFrameRateChanged;

	encoder->frameskip = 2;
	encoder->delayMs = -1;

	encoder->iwidth = GBA_VIDEO_HORIZONTAL_PIXELS;
	encoder->iheight = GBA_VIDEO_VERTICAL_PIXELS;
	encoder->numerator = VIDEO_TOTAL_LENGTH;
	encoder->denominator = GBA_ARM7TDMI_FREQUENCY;
}

void ImageMagickGIFEncoderSetParams(struct ImageMagickGIFEncoder* encoder, int frameskip, int delayMs) {
	if (ImageMagickGIFEncoderIsOpen(encoder)) {
		return;
	}
	encoder->frameskip = frameskip;
	encoder->delayMs = delayMs;
}

bool ImageMagickGIFEncoderOpen(struct ImageMagickGIFEncoder* encoder, const char* outfile) {
	MagickWandGenesis();
	encoder->wand = NewMagickWand();
	MagickSetImageFormat(encoder->wand, "GIF");
	MagickSetImageDispose(encoder->wand, PreviousDispose);
	encoder->outfile = strdup(outfile);
	encoder->frame = malloc(encoder->iwidth * encoder->iheight * 4);
	encoder->currentFrame = 0;
	return true;
}

bool ImageMagickGIFEncoderClose(struct ImageMagickGIFEncoder* encoder) {
	if (!encoder->wand) {
		return false;
	}

	MagickBooleanType success = MagickWriteImages(encoder->wand, encoder->outfile, MagickTrue);
	DestroyMagickWand(encoder->wand);
	encoder->wand = 0;
	free(encoder->outfile);
	free(encoder->frame);
	MagickWandTerminus();
	return success == MagickTrue;
}

bool ImageMagickGIFEncoderIsOpen(struct ImageMagickGIFEncoder* encoder) {
	return !!encoder->wand;
}

static void _magickPostVideoFrame(struct mAVStream* stream, const color_t* pixels, size_t stride) {
	struct ImageMagickGIFEncoder* encoder = (struct ImageMagickGIFEncoder*) stream;

	if (encoder->currentFrame % (encoder->frameskip + 1)) {
		++encoder->currentFrame;
		return;
	}

	const uint8_t* p8 = (const uint8_t*) pixels;
	size_t row;
	for (row = 0; row < encoder->iheight; ++row) {
		memcpy(&encoder->frame[row * encoder->iwidth], &p8[row * 4 * stride], encoder->iwidth * 4);
	}

	MagickConstituteImage(encoder->wand, encoder->iwidth, encoder->iheight, "RGBP", CharPixel, encoder->frame);
	uint64_t ts = encoder->currentFrame;
	uint64_t nts = encoder->currentFrame + encoder->frameskip + 1;
	if (encoder->delayMs >= 0) {
		ts *= encoder->delayMs;
		nts *= encoder->delayMs;
		ts /= 10;
		nts /= 10;
	} else {
		ts *= encoder->numerator * 100;
		nts *= encoder->numerator * 100;
		ts /= encoder->denominator;
		nts /= encoder->denominator;
	}
	MagickSetImageDelay(encoder->wand, nts - ts);
	++encoder->currentFrame;
}

static void _magickVideoDimensionsChanged(struct mAVStream* stream, unsigned width, unsigned height) {
	struct ImageMagickGIFEncoder* encoder = (struct ImageMagickGIFEncoder*) stream;
	if (encoder->iwidth == width && encoder->iheight == height) {
		return;
	}
	if (width * height > encoder->iwidth * encoder->iheight) {
		free(encoder->frame);
		encoder->frame = malloc(width * height * 4);
	}
	encoder->iwidth = width;
	encoder->iheight = height;
	encoder->frame = malloc(encoder->iwidth * encoder->iheight * 4);
}

static void _magickVideoFrameRateChanged(struct mAVStream* stream, unsigned numerator, unsigned denominator) {
	struct ImageMagickGIFEncoder* encoder = (struct ImageMagickGIFEncoder*) stream;
	encoder->numerator = numerator;
	encoder->denominator = denominator;
}
