#ifndef bit_layout_reworked
#define bit_layout_reworked


/*
the fv1 file consists of 3 parts:
	the header (FV1_HEADER)
	the framelist (frame_data * N)
	the frames (other)
see assets/bitstream.png for the same information visualized
*/

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
	unsigned long long fv1_number = 0x30726d6362315646;
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
	// 0b0100 = keyframe, see frame types, (keyframes will be decoded and encoded as C frames, but there will have to be a special flag needed only when encoding)
	// 0b0101 - 0b1110: reserved
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
// four bits of unused data, what a waste of space, but just keeping 4 bits will give a massive performance drop when the data behind it has to be realigned
struct C_FRAME {
	// information
	// 0b00000001 = changed
	// 0b00000000 = unchanged
	// 0b00000010 = split horizontally
	// 0b00000011 = split vertically
	// 0b00000100 = split 4 way squares
	// if the 5th bit is set, and the frame is split, the frame will have 'pointers' so it can be multithreaded when decompressing, if the 5th bit isn't set, the next split block can be found by calculating the read bytes from the split block before (a bit complex to explain here) 
	/* 
		the reason for this is that 'framepointers' take 8 bytes of data, 
		and having those pointers for every 16x16 block in a 1440x2560 pixel video will have 20000 framepointers for only the luminescence layer,
		if YUV420p is used, there will be a total of 30000 framepointers for one frame, multiply that by 60 for the framerate and by 64 bits for the framepointer length and it gives:
		overhead_bitrate = 30000 * 60 * 64 = 112Mbit/s for only the offsets

		having no offset pointers means no multithreading, having too much offset pointers means larger filesize,
		the encoder can decide how deep to allow multithreading, i'd say 2 layers is good enough with a maximum of 20 (4 + 4^2) offset pointers per frame,
		this allows up to 16 threads to be used to decode a frame, and an overhead of only 76.8kb/s
	*/
	// 0b00001xxx 
	char info;

	

};

enum FV_FRAMETYPES {
	UNCHANGED = 0,
	C_FRAME_ID = 1,
	R_FRAME_ID = 2,
	B_FRAME_ID = 3,
	KEYFRAME_ID = 4,
	RESERVED,
	END_OF_STREAM = 15 
};








#endif