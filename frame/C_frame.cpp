#include "C_frame.hpp"

// compares difference (not finished yet)
bool cmp_diff(AVFrame *in, AVFrame *prev, int xpos, int ypos){

	return true;
}

framelist_entry encode_c(FV1_HEADER info, AVFrame* in, AVFrame* prev){
	// split into 16x16 blocks 
	int xblocks = (info.px_x >> 4) + ((info.px_x & 15) > 0 ? 1 : 0);
	int yblocks = (info.px_y >> 4) + ((info.px_y & 15) > 0 ? 1 : 0);
	
	/*
	bool *diff_table = new bool[xblocks*yblocks];
	// compare 16x16 blocks
	for (int i = 0; i < yblocks; i++){
		for (int ii = 0; ii < xblocks; ii++){
			// multithreading can be used since arrays are thread safe in this way
			diff_table[i*xblocks + ii] = cmp_diff(in, prev, ii << 4, i << 4);

		}
	}*/

	// combine 16x16 blocks into larger shapes

	// but right now, just output the raw data since that's easier
	// only execute if YUV420P
	unsigned char *outbuf;
	int outbuf_index = 0;
	
	if (info.pix_fmt == AVPixelFormat::AV_PIX_FMT_YUV420P){
		// change to faster memory copying (memcpy or something else)
		outbuf = new unsigned char[xblocks * yblocks * 256 * 2];
		for (int i = 0; i < in->height * 2; i++){
			int output_offset = outbuf_index + i * in->height;
			

				memcpy(&outbuf[output_offset], &(in->data[0][output_offset]), in->width);
		}		
		outbuf_index += in->height * in->width;
		for (int i = 0; i < in->height; i++){
			int output_offset = outbuf_index + i * (in->height >> 1);

			memcpy(&(outbuf[output_offset]), &(in->data[1][i* (in->height >> 1)]), in->height >> 1);
		}	
		outbuf_index += (in->height >> 1) * (in->width >> 1);
		for (int i = 0; i < in->height; i++){
			int output_offset = outbuf_index + i * (in->height >> 1);
			memcpy(&(outbuf[output_offset]), &(in->data[2][i* (in->height >> 1)]), in->height >> 1);

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


void decode_c(FV1_HEADER info, std::ifstream &f_in, unsigned long long frame_pos, AVFrame* out, AVFrame* prev){
	// read from file to out
	f_in.seekg(frame_pos);

	unsigned long long pixels = info.px_x * info.px_y;
	
	
	
	if (info.pix_fmt == AVPixelFormat::AV_PIX_FMT_YUV420P){
		
	// the size of the data to read can be determined from the fv1 header and the data, even with DCT, RLE, and other compression algorithms
		f_in.read((char*)out->data[0], pixels);
		f_in.read((char*)out->data[1], pixels >> 2);
		f_in.read((char*)out->data[2], pixels >> 2);
	}

	// very unsafe bug error can happen here
	
	


}
