#ifndef bit_layout_reworked
#define bit_layout_reworked

enum px_fmts {
	YUV420P888 = 0x01
};


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

	// divide this value by thousand to get the actual fps. actual fps will range from 0.001 upwards
	// special values (might be removed later): 
	// 0xFFFFFFFFFFFFFFFE = 29.97 fps 
    unsigned long long fps;



	// how and what colors are stored (monochrome, rgb, yuv)
	/*
	0x0000 = rgb888
	0x0001= yuv420p8
	*/
	unsigned long long pix_fmt;

	// only neccesary for yuv colorspaces i think
	/*
		bt2020, bt709, etc. 

	*/
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








#endif