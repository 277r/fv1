
#ifndef FRAMESTREAM_CREATION
#define FRAMESTREAM_CREATION

#include "../bit_layout_reworked.hpp"
#include <vector>

// the encoder creates a vector of all encoded frames, and sends that to the framestream creation, which places all offset pointers based on the 
struct framelist_entry {
	int type;
	unsigned long long size;
	void *location;

};
unsigned char *create_frame_stream(std::vector<framelist_entry> input, unsigned long long &length, std::vector<unsigned long long> &f_positions);

#endif
