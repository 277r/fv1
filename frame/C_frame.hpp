#include "../libav_include.hpp"

#include "../bit_layout_reworked.hpp"
#include "../stream_creation/framestream_creation.hpp"
#include <iostream>
#include <fstream>

framelist_entry encode_c(FV1_HEADER info, AVFrame* in, AVFrame* prev);

void decode_c(FV1_HEADER info, std::ifstream &f_in, unsigned long long frame_pos, AVFrame* out, AVFrame* prev);