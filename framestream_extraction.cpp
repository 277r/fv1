#include <vector>

// extracted framestream block
struct fs_ex_block
{
	unsigned long long p1, p2;
	unsigned char t1, t2;
};

std::vector<fs_ex_block> extract_framestream(void *data, unsigned long long len)
{
	std::vector<fs_ex_block> output_data;

	if (len != 0)
	{
	}

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