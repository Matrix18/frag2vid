#ifndef _FRAG2VID_ENCODER_H_
#define _FRAG2VID_ENCODER_H_

#include <stdio.h>
#include <stdlib.h>

#include <GL/gl.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/mem.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

static void frameRGBtoYUV(uint8_t *rgb);

void encoderStart(const char *filename, int codec_id, int fps, int width, int height);

void encoderFinish();

void encodeFrame(uint8_t *rgb);

void encoderReadGL(uint8_t **rgb, GLubyte **pixels, unsigned int width, unsigned int height);

void setFramepts(unsigned int newpts);

#endif
