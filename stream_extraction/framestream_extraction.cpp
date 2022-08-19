#include "framestream_extraction.hpp"

// get the framestream location using the sizes of the quantization matrix, this might be done without a function but this seems like a good way to keep everything clean












// in development 1 by 1 frame extractor, for when it's needed (it probably won't be needed so i probably won't finish this)
class frame_extractor {
public:
	fs_ex_block current_frames;
	void get_next_2_frames(){

	}
	void set_framestream_data(void *data){

	}
private:
	unsigned long long pos = 0;
	void *data; 


};




std::vector<fs_ex_block> extract_framestream(void *data, unsigned long long len)
{
	std::vector<fs_ex_block> output_data;

	// if filesize is known, use this
	if (len != 0)
	{

	}



	// treat like a stream with an unkown size 

	bool end_of_stream = false;

	fs_ex_block tmp1;
	fs_ex_block tmp2;

	unsigned long long pos = 0;
	while (!end_of_stream)
	{

		// extract the values
		// get 4 leftmost bits by shifting 4 bits to right
		tmp1.t = (((unsigned char *)data)[pos]) >> 4;
		// get 4 rightmost bits by logically ANDing with the 4 rightmost bits
		tmp2.t = (((unsigned char *)data)[pos]) & 0b1111;
		
		pos++;

		

		// if end of stream at first frame in a block
		if (tmp1.t == 0b1111)
		{
			end_of_stream = true;
			// return now or a garbage block is appended
			return output_data;
		}

		else if (tmp2.t == 0b1111)
		{
			end_of_stream = true;			
			// retrieve first frame, which is still whole
			if (tmp1.t != 0b0000)
			{
				tmp1.p = *((unsigned long long *)(data + pos));
				pos += 8;
				
				
			}
			output_data.push_back(tmp1);
			return output_data;
			
		}

		
		else
		{

			// this can be optimized
			// extract the frames
			if (tmp1.t == 0b0000 && tmp2.t == 0b0000)
			{
				// do nothing
			}
			else if (tmp1.t != 0b0000 && tmp2.t == 0b0000)
			{
				// grab first frame location
				tmp1.p = *((unsigned long long *)(data + pos));
				pos += 8;

			}
			else if (tmp1.t == 0b0000 && tmp2.t != 0b0000){
				// grab second frame
				tmp2.p = *((unsigned long long *)(data + pos));
				pos += 8;

			}
			else {
				// grab 1st and 2nd frame
				tmp1.p = *((unsigned long long *)(data + pos));
				tmp2.p = *((unsigned long long *)(data + pos + 8));
				pos += 16;
			}


		}
		output_data.push_back(tmp1);
		output_data.push_back(tmp2);
	}


	return output_data;
}