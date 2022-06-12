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

	fs_ex_block tmp;

	unsigned long long pos = 0;
	while (!end_of_stream)
	{
		// set zero
		tmp.p1 = 0;
		tmp.p2 = 0;

		// extract the values
		// get 4 leftmost bits by shifting 4 bits to right
		tmp.t1 = (((unsigned char *)data)[pos]) >> 4;
		// get 4 rightmost bits by logically ANDing with the 4 rightmost bits
		tmp.t2 = (((unsigned char *)data)[pos]) & 0b1111;
		
		pos++;

		

		// if end of stream at first frame
		if (tmp.t1 == 0b1111)
		{
			end_of_stream = true;
		}

		else if (tmp.t2 == 0b1111)
		{
			end_of_stream = true;			
			// retrieve first frame, which is still whole
			if (tmp.t1 != 0b0000)
			{
				tmp.p1 = *((unsigned long long *)(data + pos));
				pos += 8;
				
			}
			
		}

		
		else
		{

			// this can be optimized
			// extract the frames
			if (tmp.t1 == 0b0000 && tmp.t2 == 0b0000)
			{
				// do nothing
			}
			else if (tmp.t1 != 0b0000 && tmp.t2 == 0b0000)
			{
				// grab first frame location
				tmp.p1 = *((unsigned long long *)(data + pos));
				pos += 8;

			}
			else if (tmp.t1 == 0b0000 && tmp.t2 != 0b0000){
				// grab second frame
				tmp.p2 = *((unsigned long long *)(data + pos));
				pos += 8;

			}
			else {
				// grab 1st and 2nd frame
				tmp.p1 = *((unsigned long long *)(data + pos));
				tmp.p2 = *((unsigned long long *)(data + pos + 8));
				pos += 16;
			}


		}
		output_data.push_back(tmp);
	}


	return output_data;
}