#ifndef bit_layout_reworked
#define bit_layout_reworked



struct FV1_HEADER {
	// FV1 special number
	/* 
	70
	86
	49
	98
	99
	109
	114
	48
	*/
	// ascii: "FV1bcmr0"
	unsigned long long fv1_number = 0x46563162636d7230;
	// amount of frames the video has
	// if this number is zero, frames will be seen as a stream
	unsigned long long frames[2];
	
	// amount of pixels
	unsigned long long px_x;
	unsigned long long px_y;

	// fps numerator and denumerator
	// actual fps = num/den;
    unsigned long long fps_num;
	unsigned long long fps_den;



	// how and what colors are stored (monochrome, rgb, yuv)
	// uses the libav defined pixelformats in pixfmt.h
	unsigned long long pix_fmt;

	// only neccesary for yuv colorspaces i think
	// also uses the libav defined colorspaces in pixfmt.h
	unsigned short colorspace;

};



// stores data for 2 frames. is stored to disk as one byte 
struct frame_data {
	// states and meanings:
	// 0b0000 = unchanged (can be called U frame but should not be called a frame), same frame as last frame
	// note to self: possible segfault issue if unallocated pointer and C frame without having a C frame before
	// 0b0001 = C frame, see frame types 
	// 0b0010 = R frame, see frame types
	// 0b0011 = B frame, see frame types
	// 0b0100 - 0b1110: reserved
	// 0b1111 = end of stream (last frame)
	int frame1 : 4;
	int frame2 : 4;
	
};

struct frame_pointer {
	// position in the file, NOT RELATIVE to the current position
	unsigned long long pos;
};


// general stream info: 
/*
	every stream starts with the FV1 header, after that, it has the frame stream, the data stream starts after the frame stream. this can be seen in assets/bitstream.png



*/

// the next part will have frame types and their inner data parts

// a C frame only stores change, it has only a few bits of information and then it has 'pointers' that point to either the data itself, or a CTU
struct C_FRAME {
	// information
	// 0b00000001 = changed
	// 0b00000000 = unchanged
	// 0b00000010 = split horizontally
	// 0b00000011 = split vertically
	// 0b00000100 = split 4 way squares
	char info;

	

};

enum FV_FRAMETYPES {
	UNCHANGED = 0,
	C_FRAME_ID = 1,
	R_FRAME_ID = 2,
	B_FRAME_ID = 3,
	RESERVED,
	END_OF_STREAM = 15 
};








#endif