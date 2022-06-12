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

#include "stream_creation/framestream_creation.hpp"
#include "bit_layout_reworked.hpp"
#include "frame/C_frame.hpp"
#include <iostream>
#include <string>
#include <fstream>



typedef struct StreamingParams {
  char copy_video;
  char copy_audio;
  char *output_extension;
  char *muxer_opt_key;
  char *muxer_opt_value;
  char *video_codec;
  char *audio_codec;
  char *codec_priv_key;
  char *codec_priv_value;
} StreamingParams;

typedef struct StreamingContext {
  AVFormatContext *avfc;
  AVCodec *video_avc;
  AVCodec *audio_avc;
  AVStream *video_avs;
  AVStream *audio_avs;
  AVCodecContext *video_avcc;
  AVCodecContext *audio_avcc;
  int video_index;
  int audio_index;
  char *filename;
} StreamingContext;

int encode(char* infile, char* outfile);

