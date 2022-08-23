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
		// changed to faster memory copying (memcpy or something else)
		outbuf = new unsigned char[xblocks * yblocks * 256 * 2];
		for (int i = 0; i < in->height; i++){
			
			int output_offset = outbuf_index + i * in->width;
			int input_offset = i * in->linesize[0];

				memcpy(&outbuf[output_offset], &(in->data[0][input_offset]), in->width);
		}		

		outbuf_index += in->height * in->width;
		for (int i = 0; i < in->height >> 1; i++){
			int output_offset = outbuf_index + i * (in->width >> 1);
			int input_offset = i * in->linesize[1];

			memcpy(&(outbuf[output_offset]), &(in->data[1][input_offset]), in->width >> 1);
		}	
		outbuf_index += (in->height >> 1) * (in->width >> 1);
		for (int i = 0; i < in->height >> 1; i++){
			int output_offset = outbuf_index + i * (in->width >> 1);
			int input_offset = i * in->linesize[2];

			memcpy(&(outbuf[output_offset]), &(in->data[2][input_offset]), in->width >> 1);

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
		if (out->linesize[0] == info.px_x){

			f_in.read((char*)out->data[0], pixels);
			f_in.read((char*)out->data[1], pixels >> 2);
			f_in.read((char*)out->data[2], pixels >> 2);
		}
		// read lines seperately
		else {
			// for every horizontal line
			for (int i = 0; i < info.px_y; i++){
				f_in.read((char*)&out->data[0][out->linesize[0] * i], info.px_x);
			}
			// uv planes at half the size
			for (int i = 0; i < info.px_y >> 1; i++){
				f_in.read((char*)&out->data[1][out->linesize[1] * i], info.px_x >> 1);
			}
			for (int i = 0; i < info.px_y >> 1; i++){
				f_in.read((char*)&out->data[2][out->linesize[2] * i], info.px_x >> 1);
			}
		}

	}

	
	


}


int test_c_frame(FV1_HEADER info, AVFrame *in, AVFrame *prev, int quality){
	if (info.pix_fmt == AVPixelFormat::AV_PIX_FMT_YUV420P){
		// divide into blocks, test change
		// do for every layer

		
	}

	else std::cout << "error: unsupported pixelformat\n";

	return 0;

}
