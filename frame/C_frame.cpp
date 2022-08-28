#include "C_frame.hpp"

// compares difference (not finished yet)
bool cmp_diff(AVFrame *in, AVFrame *prev, int xpos, int ypos){

	return true;
}

// put in C_frame.cpp so it won't be in the same file with functions that can be called, since this is only needed in the C frame encode/decode part and might make people mix up the different structs
// the C frame struct when in memory, which is different than the one from bitlayout_reworked.hpp, because this one actually contains pointers 
struct C_FRAME_MEM {
	char info;
	// left top, right top, left bottom, right bottom parts
	C_FRAME_MEM *lt, *rt, *lb, *rb;
	// sizes of data
	int lts, rts, lbs, rbs;
	// in case of info == 1, use the data object 
	void *data;
	int size;
	

};

// transform raw pixel data into usable data
void *encode_block(FV1_HEADER info, AVFrame*in, int xbpos, int ybpos, int &size){

}

// no need for previous frame since diff table has been created
// xstart ystart xlen ylen are used for the recursion, those should just be fed with {0,0, xblocks, yblocks}
C_FRAME_MEM create_tree_and_encode(FV1_HEADER info, AVFrame *in,bool **diff_table, int xstart, int ystart, int xlen, int ylen){
	
	// the part that gets returned
	C_FRAME_MEM ret;
	

	// all split parts of frames etc
	C_FRAME_MEM plt, prt, plb, prb;
	int part_count;

	// deepest part of block reached when both x and y length equal zero, most complex function
	if (xlen == 1 && ylen == 1){
		part_count = 1;
		ret.info = 1;
		ret.data = encode_block(info, in, xstart, ystart, ret.size);
		// probably safe to return right here

	}
	
	// split into 2-4 parts
	
	// next 2 else if blocks are made so that videos that don't have xblocks = yblocks work without needing any more storage space, it might be a bit complicated to explain 
	else if (xlen == 1){
		// set split direction
		ret.info = 0b00000011;
		part_count = 2;
		int new_ylen = (ylen >> 1);
		int new_ylen2 = (ylen >> 1) + (ylen & 1);

		// x length stays the same so it's fed through to the recurring function, y length has to be halfed,
		// (unimportant for the explanation): the floored half will be the top half, and the ceiled half will be the bottom half, so that all blocks get used even when theres an uneven amount like 7 blocks it will be split into 3 and 4
		// top half starts at the current place but ylen is changed
		plt = create_tree_and_encode(info, in, diff_table, xstart, ystart, xlen ,new_ylen);
		// by by adding ystart + new_ylen it will start where the top half ends, and using new_ylen2 (in case input ylen == 7) will make it start at 3 and have length 4 
		prb = create_tree_and_encode(info, in, diff_table, xstart, ystart + new_ylen, xlen, new_ylen2);

	}

	else if (ylen == 1){
		ret.info = 0b00000010;
		part_count = 2;
		int new_xlen = (xlen >> 1);
		int new_xlen2 = (xlen >> 1) + (xlen & 1);

		plt = create_tree_and_encode(info, in, diff_table, xstart, ystart, new_xlen ,ylen);
		prb = create_tree_and_encode(info, in, diff_table, xstart + new_xlen, ystart, new_xlen2, ylen);
	}
	
	else {
		// a bit more complex but should do the same in 4 blocks
		part_count = 4;
		int new_xlen = (xlen >> 1);
		int new_xlen2 = (xlen >> 1) + (xlen & 1);		
		int new_ylen = (ylen >> 1) + (ylen & 1);
		int new_ylen2 = (ylen >> 1) + (ylen & 1);

		plt = create_tree_and_encode(info, in, diff_table, xstart, ystart, new_xlen ,new_ylen);
		prt = create_tree_and_encode(info, in, diff_table, xstart + new_xlen, ystart, new_xlen2, new_ylen);
		plb = create_tree_and_encode(info, in, diff_table, xstart, ystart + new_ylen, new_xlen, new_ylen2);
		prb = create_tree_and_encode(info, in, diff_table, xstart + new_xlen, ystart + new_ylen, new_xlen2, new_ylen2);

	}

	// check parts after receiving results, combine if possible

	// (depending on part_count):
	// if all parts.info == 1, add all raw data to save overhead, and return
	// else
	// for parts:
	/*
		if part[n].info == 1, add part[n].size
		if part[n].info == hsplit, add both parts.size

	*/ 


	if (part_count == 2){
		// if all parts are just raw data, just store the raw data, the decoder will know how to figure out where to place what data based on the recursion level and current position in the decoding function
		if (plt.info == 1 && prb.info == 1){
			// can be optimized
			// allocate data for both sizes combined
			ret.data = new unsigned char[plt.size + prb.size];
			// copy first buffer, copy second buffer and put right after first buffer
			memcpy(ret.data, plt.data, plt.size);
			memcpy(ret.data + plt.size, prt.data, prt.size);
			ret.info = 0b00000001;
			return ret;
		}
		else {
			// check every other possibility
		}
	}
	// do the same for 4 parts
	if (part_count == 4){

	}
	


}

framelist_entry encode_c(FV1_HEADER info, AVFrame* in, AVFrame* prev){
	// split into 16x16 blocks 
	int xblocks = (info.px_x >> 4) + ((info.px_x & 15) > 0 ? 1 : 0);
	int yblocks = (info.px_y >> 4) + ((info.px_y & 15) > 0 ? 1 : 0);
	

	
	bool **diff_table = new bool*[yblocks];
	for (int i = 0; i < yblocks; i++){
		diff_table[i] = new bool[xblocks];
	}

	// compare 16x16 blocks
	for (int i = 0; i < yblocks; i++){
		for (int ii = 0; ii < xblocks; ii++){
			// multithreading can be used since arrays are thread safe in this way
			diff_table[i][ii] = cmp_diff(in, prev, ii << 4, i << 4);

		}
	}

	// combine 16x16 blocks into larger shapes
	

	// delete diff table after having created the Cframe tree
	for (int i = 0; i < yblocks; i++){
		delete[] diff_table[i];
	}
	
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
		// do for every layer, (i.e. (R,G,B), or (Y,U,V))

	}


	else std::cout << "error: unsupported pixelformat\n";

	return 0;

}
