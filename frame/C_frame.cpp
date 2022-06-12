#include "C_frame.hpp"

// compares difference (not finished yet)
bool cmp_diff(AVFrame *in, AVFrame *prev, int xpos, int ypos){

	return true;
}

framelist_entry encode_c(FV1_HEADER info, AVFrame* in, AVFrame* prev){
	// split into 16x16 blocks 
	int xblocks = (info.px_x >> 4) + ((info.px_x & 15) > 0 ? 1 : 0);
	int yblocks = (info.px_y >> 4) + ((info.px_y & 15) > 0 ? 1 : 0);
	
	bool *diff_table = new bool[xblocks*yblocks];
	// compare 16x16 blocks
	for (int i = 0; i < yblocks; i++){
		for (int ii = 0; ii < xblocks; ii++){
			// multithreading can be used since arrays are thread safe in this way
			diff_table[i*xblocks + ii] = cmp_diff(in, prev, ii << 4, i << 4);

		}
	}

	// combine 16x16 blocks into larger shapes

	// but right now, just output the raw data since that's easier
	// only execute if YUV420P
	unsigned char *outbuf;
	int outbuf_index = 0;
	
	if (info.pix_fmt == AVPixelFormat::AV_PIX_FMT_YUV420P){

		outbuf = new unsigned char[xblocks * yblocks * 256 * 2];
		for (int i = 0; i < in->height; i++){
			for (int ii = 0; ii < in->width; ii++){
				outbuf[outbuf_index + i * in->height + ii] = in->data[0][i* in->height + ii];
			}
		}		
		outbuf_index += in->height * in->width;
		for (int i = 0; i < in->height >> 1; i++){
			for (int ii = 0; ii < in->width >> 1; ii++){
				outbuf[outbuf_index + i * (in->height >> 1) + ii] = in->data[1][i * (in->height >> 1) + ii];
			}
		}	
		outbuf_index += (in->height >> 1) * (in->width >> 1);
		for (int i = 0; i < in->height >> 1; i++){
			for (int ii = 0; ii < in->width >> 1; ii++){
				outbuf[outbuf_index + i * (in->height >> 1) + ii] = in->data[2][i * (in->height >> 1) + ii];
			}
		}	
		outbuf_index += (in->height >> 1) * (in->width >> 1);

	}

	else {
		std::cout << "error: pixelformat not implemented yet\n";
		return {0, 0, 0};
	}

	framelist_entry j;
	j.location = outbuf;
	j.size = outbuf_index;
	j.type = FV_FRAMETYPES::C_FRAME_ID;
	
	return j;

}
