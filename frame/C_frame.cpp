#include "C_frame.hpp"

// compares difference (not finished yet)
bool cmp_diff(AVFrame *in, AVFrame *prev, int xpos, int ypos)
{

	return true;
}

// put in C_frame.cpp so it won't be in the same file with functions that can be called, since this is only needed in the C frame encode/decode part and might make people mix up the different structs
// the C frame struct when in memory, which is different than the one from bitlayout_reworked.hpp, because this one actually contains pointers
struct C_FRAME_MEM
{
	char info;
	// left top, right top, left bottom, right bottom parts
	C_FRAME_MEM *lt, *rt, *lb, *rb;
	// in case of info == 1, use the data object
	void *data;
	int size;
};

void free_c_frame_mem_tree(C_FRAME_MEM *p)
{
	// delete all subdata recursively
	if (p->info == 0b00)
	{
		delete[] p;
		return;
	}
	if (p->info == 0b01)
	{
		free(p->data);
		delete[] p;
		return;
	}
	// must be updated if the diagonal split is added
	if (p->info == 0b10 || p->info == 0b11)
	{
		free_c_frame_mem_tree(p->lt);
		free_c_frame_mem_tree(p->rb);
		delete[] p;
		return;
	}
	else
	{
		free_c_frame_mem_tree(p->lt);
		free_c_frame_mem_tree(p->rt);
		free_c_frame_mem_tree(p->lb);
		free_c_frame_mem_tree(p->rb);
		delete[] p;
		return;
	}
}

// transform raw pixel data into usable data
void *encode_block(FV1_HEADER info, AVFrame *in, int xbpos, int ybpos, int &size)
{
	// add Y layer, add U layer, add V layer
	// later, use transforms and quantization instead, so those need to return size too;

	if (info.pix_fmt == AVPixelFormat::AV_PIX_FMT_YUV420P)
	{	
		// max size for 8 bit yuv 420p video
		unsigned char *retdata = new unsigned char[16 * 16 + 8 * 8 + 8 * 8];
		int retdata_pos = 0;

		int xpixelpos = xbpos * 16;
		int ypixelpos = ybpos * 16;

		for (int i = ypixelpos; i < ypixelpos + 16 && i < in->height; i++)
		{
			for (int ii = xpixelpos; ii < xpixelpos + 16 && ii < in->width; ii++)
			{
				// copy data
				retdata[retdata_pos] = in->data[0][ii + i * in->linesize[0]];
				retdata_pos++;
			}
		}
		// UV planes have a much smaller size
		xpixelpos = xbpos * 8;
		ypixelpos = ybpos * 8;

		for (int i = ypixelpos; i < ypixelpos + 8 && i < in->height; i++)
		{
			for (int ii = xpixelpos; ii < xpixelpos + 8 && ii < in->width; ii++)
			{
				// copy data
				retdata[retdata_pos] = in->data[1][ii + i * in->linesize[1]];
				retdata_pos++;
			}
		}

		for (int i = ypixelpos; i < ypixelpos + 8 && i < in->height; i++)
		{
			for (int ii = xpixelpos; ii < xpixelpos + 8 && ii < in->width; ii++)
			{
				// copy data
				retdata[retdata_pos] = in->data[2][ii + i * in->linesize[2]];
				retdata_pos++;
			}
		}

		size = retdata_pos;
		return (void *)retdata;
	}
}

// function used for create_tree_and_encode(...);
C_FRAME_MEM *combine_2_blocks(C_FRAME_MEM *plt, C_FRAME_MEM *prb, int split_direction)
{
	// split direction:
	// 0b10 == vertical split
	// 0b11 == horizontal split
	/* possible combinations of plt->info and prb->info:
			plt == 0, prb == 0
			plt == 0, prb == 1
			plt == 1, prb == 0
			plt == 1, prb == 1

			also possible: plt and prb having other values than 0 and 1, in that case just return a C_FRAME_MEM which contains the plt and prb
		*/
	C_FRAME_MEM *ret = new C_FRAME_MEM;
	// if all parts are just raw data, just store the raw data, the decoder will know how to figure out where to place what data based on the recursion level and current position in the decoding function
	if (plt->info == 1 && prb->info == 1)
	{
		
		// can be optimized
		// allocate data for both sizes combined
		ret->size = plt->size + prb->size;
		ret->data = new unsigned char[plt->size + prb->size];
		// copy first buffer, copy second buffer and put right after first buffer
		memcpy(ret->data, plt->data, plt->size);
		memcpy(ret->data + plt->size, prb->data, prb->size);
		// after copying memory, delete
		free(plt->data);
		free(prb->data);
		delete[] plt;
		delete[] prb;
		
		ret->info = 0b00000001;
		return ret;

	}

	else if (plt->info == 0 && prb->info == 0)
	{
		// there should be no leak here but might need to free plt->data and prb->data
		delete[] plt;
		delete[] prb;
		ret->info = 0b00000000;

		return ret;
	}

	// combination 2,3, can't combine
	else
	{
		ret->info = split_direction;
		ret->lt = plt;
		ret->rb = prb;
		
		return ret;
	}
}

// no need for previous frame since diff table has been created
// xstart ystart xlen ylen are used for the recursion, those should just be fed with {0,0, xblocks, yblocks}
C_FRAME_MEM *create_tree_and_encode(FV1_HEADER info, AVFrame *in, bool **diff_table, int xstart, int ystart, int xlen, int ylen)
{

	// the part that gets returned
	C_FRAME_MEM *ret = new C_FRAME_MEM;

	// all split parts of frames etc
	C_FRAME_MEM *plt = new C_FRAME_MEM;
	C_FRAME_MEM *prt = new C_FRAME_MEM;
	C_FRAME_MEM *plb = new C_FRAME_MEM;
	C_FRAME_MEM *prb = new C_FRAME_MEM;
	int part_count;

	// this block is allowed to return, because it does not have much multiple cases like horizontal split and vertical split being able to use the same functions for return data
	// deepest part of block reached when both x and y length equal zero, most complex function
	if (xlen == 1 && ylen == 1)
	{
		// 4 parts won't be returned so delete
		delete[] plt;
		delete[] prt;
		delete[] plb;
		delete[] prb;

		if (diff_table[ystart][xstart] == 0)
		{
			ret->info = 0;
			ret->size = 0;
			return ret;
		}
		else
		{
			ret->info = 1;
			// this is where the actual data comes from
			ret->data = encode_block(info, in, xstart, ystart, ret->size);
			return ret;
		}
	}

	// split into 2-4 parts

	// next 2 else if blocks are made so that videos that don't have xblocks = yblocks work without needing any more storage space, it might be a bit complicated to explain
	else if (xlen == 1)
	{
		// set split direction
		ret->info = 0b00000011;
		part_count = 2;
		int new_ylen = (ylen >> 1);
		int new_ylen2 = (ylen >> 1) + (ylen & 1);

		// x length stays the same so it's fed through to the recurring function, y length has to be halfed,
		// (unimportant for the explanation): the floored half will be the top half, and the ceiled half will be the bottom half, so that all blocks get used even when theres an uneven amount like 7 blocks it will be split into 3 and 4
		// top half starts at the current place but ylen is changed

		plt = create_tree_and_encode(info, in, diff_table, xstart, ystart, xlen, new_ylen);

		// by by adding ystart + new_ylen it will start where the top half ends, and using new_ylen2 (in case input ylen == 7) will make it start at 3 and have length 4
		prb = create_tree_and_encode(info, in, diff_table, xstart, ystart + new_ylen, xlen, new_ylen2);
	}

	else if (ylen == 1)
	{
		ret->info = 0b00000010;
		part_count = 2;
		int new_xlen = (xlen >> 1);
		int new_xlen2 = (xlen >> 1) + (xlen & 1);

		plt = create_tree_and_encode(info, in, diff_table, xstart, ystart, new_xlen, ylen);
		prb = create_tree_and_encode(info, in, diff_table, xstart + new_xlen, ystart, new_xlen2, ylen);
	}

	else
	{

		// a bit more complex but should do the same in 4 blocks
		part_count = 4;
		int new_xlen = (xlen >> 1);
		int new_xlen2 = (xlen >> 1) + (xlen & 1);
		int new_ylen = (ylen >> 1);
		int new_ylen2 = (ylen >> 1) + (ylen & 1);

		plt = create_tree_and_encode(info, in, diff_table, xstart, ystart, new_xlen, new_ylen);
		prt = create_tree_and_encode(info, in, diff_table, xstart + new_xlen, ystart, new_xlen2, new_ylen);
		plb = create_tree_and_encode(info, in, diff_table, xstart, ystart + new_ylen, new_xlen, new_ylen2);
		prb = create_tree_and_encode(info, in, diff_table, xstart + new_xlen, ystart + new_ylen, new_xlen2, new_ylen2);

	}
	// check parts after receiving results, combine if possible

	// 2 way split
	if (part_count == 2)
	{
		// prt and plb won't be returned, so delete
		delete[] prt;
		delete[] plb;
		delete[] ret;
		return combine_2_blocks(plt, prb, ret->info);
	}
	// do the same for 4 parts
	// this will be a pain, because the 4 (2x2) parts can also form 2 split blocks (a horizontal and vertical split, with either horizontal first or vertical first)
	// look at assets/c_tree_4_notes.png (warning: dutch)
	if (part_count == 4)
	{

		// not sure if switch could be used here due to the multiple values

		// all zero
		if (plt->info == 0 && prt->info == 0 && plb->info == 0 && prb->info == 0)
		{
			// 4 parts won't be returned so delete
			delete[] plt;
			delete[] prt;
			delete[] plb;
			delete[] prb;
			ret->info = 0b00000000;
			return ret;
		}

		// all one
		if (plt->info == 1 && prt->info == 1 && plb->info == 1 && prb->info == 1)
		{

			std::cout << "data: " << plt->data << ", " << prt->data << ", " << plb->data << ", " << prb->data << std::endl ;
			std::cout << "sizes: " << plt->size << ", " << prt->size << ", " << plb->size << ", " << prb->size << std::endl ;
			std::cout << "info: " << (int) plt->info << ", " <<  (int)prt->info << ", " <<  (int)plb->info << ", " <<  (int)prb->info << std::endl ;
			
			// can be optimized
			// allocate data for all sizes combined
			ret->data = new unsigned char[plt->size + prt->size + plb->size + prb->size];
			ret->size = plt->size + prt->size + plb->size + prb->size;

			// copy first buffer, copy second buffer and put right after first buffer
			memcpy(ret->data, plt->data, plt->size);
			memcpy(ret->data + plt->size, prt->data, prt->size);
			memcpy(ret->data + plt->size + prt->size, plb->data, plb->size);
			memcpy(ret->data + plt->size + prt->size + plb->size, prb->data, prb->size);
			// 4 parts won't be returned so delete, after memcopying all values;
			free(plt->data);
			free(prt->data);
			free(plb->data);
			free(prb->data);
			delete[] plt;
			delete[] prt;
			delete[] plb;
			delete[] prb;
			ret->info = 0b00000001;
			return ret;
		}


		// vertical check (left side 00, left side 11, right side 00, right side 11)
		if (plt->info == 0 && plb->info == 0)
		{
			delete[] ret;

			return combine_2_blocks(combine_2_blocks(plt, plb, 0b11), combine_2_blocks(prt, prb, 0b11), 0b10);
		}
		if (plt->info == 1 && plb->info == 1)
		{
			delete[] ret;
			return combine_2_blocks(combine_2_blocks(plt, plb, 0b11), combine_2_blocks(prt, prb, 0b11), 0b10);
		}
		if (prt->info == 0 && prb->info == 0)
		{
			delete[] ret;
			return combine_2_blocks(combine_2_blocks(plt, plb, 0b11), combine_2_blocks(prt, prb, 0b11), 0b10);
		}
		if (prt->info == 1 && prb->info == 1)
		{
			delete[] ret;
			return combine_2_blocks(combine_2_blocks(plt, plb, 0b11), combine_2_blocks(prt, prb, 0b11), 0b10);
		}

		// horizontal check (top side 00, top side 11, bottom side 00, bottom side 11)

		if (plt->info == 0 && prt->info == 0)
		{
			delete[] ret;
			return combine_2_blocks(combine_2_blocks(plt, prt, 0b10), combine_2_blocks(plb, prb, 0b10), 0b11);
		}
		if (plt->info == 1 && prt->info == 1)
		{
			delete[] ret;
			return combine_2_blocks(combine_2_blocks(plt, prt, 0b10), combine_2_blocks(plb, prb, 0b10), 0b11);
		}
		if (plb->info == 0 && prb->info == 0)
		{
			delete[] ret;
			return combine_2_blocks(combine_2_blocks(plt, prt, 0b10), combine_2_blocks(plb, prb, 0b10), 0b11);
		}
		if (plb->info == 1 && prb->info == 1)
		{
			delete[] ret;
			return combine_2_blocks(combine_2_blocks(plt, prt, 0b10), combine_2_blocks(plb, prb, 0b10), 0b11);
		}

		// possible compression optimization: same as horizontal check but instead of lefttop,righttop-leftbottom,rightbottom. use order lefttop,rightbottom-leftbottom,righttop. will take an extra bit but since there's still 4+ available in the info byte it doesn't matter much

		// and else, just put all pointers into ret and return with info == 4 way split

		ret->info = 0b00000100;
		ret->lt = plt;
		ret->rt = prt;
		ret->lb = plb;
		ret->rb = prb;
		return ret;
	}
	else
	{
		// error
		std::cout << "error at: " << __LINE__ << " in file: " << __FILE__ << "\n";
		exit(-1);
	}
}

framelist_entry encode_c(FV1_HEADER info, AVFrame *in, AVFrame *prev, int opts)
{
	// split into 16x16 blocks
	int xblocks = (info.px_x >> 4) + ((info.px_x & 15) > 0 ? 1 : 0);
	int yblocks = (info.px_y >> 4) + ((info.px_y & 15) > 0 ? 1 : 0);

	bool **diff_table = new bool *[yblocks];
	for (int i = 0; i < yblocks; i++)
	{
		diff_table[i] = new bool[xblocks];
	}

	// if opts &1, it's supposed to be encoded as keyframe, all image data must be reencoded
	if (opts & 1)
	{
		for (int i = 0; i < yblocks; i++)
		{
			for (int ii = 0; ii < xblocks; ii++)
			{
				diff_table[i][ii] = 1;
			}
		}
	}
	else
	{

		// compare 16x16 blocks
		for (int i = 0; i < yblocks; i++)
		{
			for (int ii = 0; ii < xblocks; ii++)
			{
				// multithreading can be used since arrays are thread safe in this way
				diff_table[i][ii] = cmp_diff(in, prev, ii, i);
			}
		}
	}

	// combine 16x16 blocks into larger shapes
	C_FRAME_MEM *p = create_tree_and_encode(info, in, diff_table, 0, 0, xblocks, yblocks);

	std::cout << "create_tree_and_encode finished\n";

	// delete diff table after having created the Cframe tree
	for (int i = 0; i < yblocks; i++)
	{
		delete[] diff_table[i];
	}

	// but right now, just output the raw data since that's easier
	// only execute if YUV420P
	unsigned char *outbuf;
	int outbuf_index = 0;

	if (info.pix_fmt == AVPixelFormat::AV_PIX_FMT_YUV420P)
	{
		// changed to faster memory copying (memcpy or something else)
		outbuf = new unsigned char[xblocks * yblocks * 256 * 2];
		for (int i = 0; i < in->height; i++)
		{

			int output_offset = outbuf_index + i * in->width;
			int input_offset = i * in->linesize[0];

			memcpy(&outbuf[output_offset], &(in->data[0][input_offset]), in->width);
		}

		outbuf_index += in->height * in->width;
		for (int i = 0; i < in->height >> 1; i++)
		{
			int output_offset = outbuf_index + i * (in->width >> 1);
			int input_offset = i * in->linesize[1];

			memcpy(&(outbuf[output_offset]), &(in->data[1][input_offset]), in->width >> 1);
		}
		outbuf_index += (in->height >> 1) * (in->width >> 1);
		for (int i = 0; i < in->height >> 1; i++)
		{
			int output_offset = outbuf_index + i * (in->width >> 1);
			int input_offset = i * in->linesize[2];

			memcpy(&(outbuf[output_offset]), &(in->data[2][input_offset]), in->width >> 1);
		}
		outbuf_index += (in->height >> 1) * (in->width >> 1);
	}

	else
	{
		std::cout << "error: pixelformat not implemented yet\n";
		return {0, 0, 0};
	}

	framelist_entry j;
	j.location = outbuf;
	j.size = outbuf_index;
	j.type = FV_FRAMETYPES::C_FRAME_ID;

	return j;
}

void decode_c(FV1_HEADER info, std::ifstream &f_in, unsigned long long frame_pos, AVFrame *out, AVFrame *prev)
{
	// read from file to out
	f_in.seekg(frame_pos);

	unsigned long long pixels = info.px_x * info.px_y;

	if (info.pix_fmt == AVPixelFormat::AV_PIX_FMT_YUV420P)
	{

		// the size of the data to read can be determined from the fv1 header and the data, even with DCT, RLE, and other compression algorithms
		if (out->linesize[0] == info.px_x)
		{

			f_in.read((char *)out->data[0], pixels);
			f_in.read((char *)out->data[1], pixels >> 2);
			f_in.read((char *)out->data[2], pixels >> 2);
		}
		// read lines seperately
		else
		{
			// for every horizontal line
			for (int i = 0; i < info.px_y; i++)
			{
				f_in.read((char *)&out->data[0][out->linesize[0] * i], info.px_x);
			}
			// uv planes at half the size
			for (int i = 0; i < info.px_y >> 1; i++)
			{
				f_in.read((char *)&out->data[1][out->linesize[1] * i], info.px_x >> 1);
			}
			for (int i = 0; i < info.px_y >> 1; i++)
			{
				f_in.read((char *)&out->data[2][out->linesize[2] * i], info.px_x >> 1);
			}
		}
	}
}

int test_c_frame(FV1_HEADER info, AVFrame *in, AVFrame *prev, int quality)
{
	if (info.pix_fmt == AVPixelFormat::AV_PIX_FMT_YUV420P)
	{
		// divide into blocks, test change
		// do for every layer, (i.e. (R,G,B), or (Y,U,V))
	}

	else
		std::cout << "error: unsupported pixelformat\n";

	return 0;
}
