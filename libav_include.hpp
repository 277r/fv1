#ifndef LIBAV_INCLUDE
#define LIBAV_INCLUDE
extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libavutil/timestamp.h>
	#include <stdio.h>
	#include <stdarg.h>
	#include <stdlib.h>
	#include <libavutil/opt.h>
	#include <string.h>
	#include <inttypes.h>
}


struct StreamingContext {
  AVFormatContext *avfc;
  AVCodec *video_avc;
  AVStream *video_avs;
  AVCodecContext *video_avcc;
  char *filename;
};

#endif