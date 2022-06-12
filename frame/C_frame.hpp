

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

#include "../bit_layout_reworked.hpp"
#include "../stream_creation/framestream_creation.hpp"
#include <iostream>


framelist_entry encode_c(FV1_HEADER info, AVFrame* in, AVFrame* prev);
