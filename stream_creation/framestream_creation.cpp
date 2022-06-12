#include "framestream_creation.hpp"
// yes mixing raw pointers with vectors, reason is that raw data is useful when writing to the disk
unsigned char *create_frame_stream(std::vector<framelist_entry> input, unsigned long long &length){
	// malloc ((input.size() + 1) >> 1) * 17 bytes for the output, since that is longer than the longest theoretical frame stream can get (exact calculations hard, might be a mathematical ERROR of me)
	//output = malloc(...);

	unsigned char *output = new unsigned char[((input.size() >> 1) + 1) * 17]; 

	// length of the output data
	length = 0;
	for (int i = 0; i < (input.size() >> 1); i++){
		unsigned char info = ((input[2*i].type << 4) & 240) + (input[2*i + 1].type & 15);
		
		// add info byte to output
		output[length] = info;
		// info is one byte in length
		length += sizeof(unsigned char);

		
		
		// depending on the frame types, copy data
		
		// if both changed
		if (input[2 * i].type != FV_FRAMETYPES::UNCHANGED && input[2 * i + 1].type != FV_FRAMETYPES::UNCHANGED){
			* (unsigned long long*) (output + length) = 0;
			length += sizeof(unsigned long long);
			* (unsigned long long*) (output + length) = 0;
			length += sizeof(unsigned long long);
		}

		// if both unchanged
		else if (input[2 * i].type == FV_FRAMETYPES::UNCHANGED && input[2 * i + 1].type == FV_FRAMETYPES::UNCHANGED){

		}
		// if 2i changed 
		else if (input[2 * i + 1].type == FV_FRAMETYPES::UNCHANGED){
			* (unsigned long long*) (output + length) = 0;
			length += sizeof(unsigned long long);
		}

		// if 2i+1 changed
		else if (input[2 * i].type == FV_FRAMETYPES::UNCHANGED){
			* (unsigned long long*) (output + length) = 0;
			length += sizeof(unsigned long long);
		}

	}

	// if uneven frame count, handle here, append stream end to last info block 
	// everything's function inside this block is described in the block above
	if (input.size() & 1){
		unsigned char info = ((input[input.size()-1].type << 4) & 240) | FV_FRAMETYPES::END_OF_STREAM;

		output[length] = info;
		length += sizeof(unsigned char);

		if (input[input.size()-1].type == FV_FRAMETYPES::UNCHANGED){
			
		}
		
		else {
			* (unsigned long long*) (output + length) = 0;
			length += sizeof(unsigned long long);
		}


	}
	// create stream end in a new info block
	else {
		unsigned char info = (FV_FRAMETYPES::END_OF_STREAM << 4);
		output[length] = info;
		length += sizeof(unsigned char);

	}


	return output;
}