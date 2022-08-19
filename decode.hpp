#include "libav_include.hpp"
#include "stream_extraction/framestream_extraction.hpp"
#include "bit_layout_reworked.hpp"
#include "frame/C_frame.hpp"
#include <iostream>
#include <string>
#include <fstream>


int decode_frame(FV1_HEADER video_info, std::ifstream &f_in, unsigned long long frame_location_abs, AVFrame *frame_out, AVFrame *prev_frame, unsigned char frame_info);

int decode(char* infile, char* outfile, char *encname);


