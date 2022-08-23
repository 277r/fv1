// all documentation about the rle algorithm can be found on https://github.com/277r/simpleRLE
#include "rle.hpp"
struct block {
	// set to highest prime number from 0-255, this might decrease output sizes when quantizizing
    unsigned char sign = 251;
    unsigned char amount;
    unsigned char character;
};

// returns pointer to the new data, with size of bytes_encoded
unsigned char* rle_encode(unsigned char*inbuf, int bytes_to_encode, int &bytes_encoded_virtual, int quality){
	// theoretical max size
	unsigned char* return_data = new unsigned char[bytes_to_encode*3];
	bytes_encoded_virtual = 0;
	int bytes_encoded = 0;

	// set all 251s to 252 if the input quality is 95% or lower
	if (quality <= 95){

		for (int i = 0; i < bytes_to_encode; i++){
			// if sign as input
			if (inbuf[i] == 251){
				inbuf[i] = 252;
			}
		}
	}

	// actually encode the data
	for (int i = 0; i < bytes_to_encode; i++){
		if (inbuf[i] == 251){
			int ii = 0;

			while (i+ii < bytes_to_encode && inbuf[i] == inbuf[i+ii] && ii < 255){
				ii++;
			}


			// add block to output
			return_data[bytes_encoded_virtual] = 251;
			bytes_encoded_virtual++;
			return_data[bytes_encoded_virtual] = ii;
			bytes_encoded_virtual++;
			return_data[bytes_encoded_virtual] = 251;
			bytes_encoded_virtual++;

			bytes_encoded += ii;
			ii--;
			i+= ii;

		}

		else if (i + 3 < bytes_to_encode && inbuf[i] == inbuf[i+1] && inbuf[i+1] == inbuf[i+2] && inbuf[i+2] == inbuf[i+3]){
			int ii = 0;

			while (i+ii < bytes_to_encode && inbuf[i] == inbuf[i+ii] && ii < 255){
				ii++;
			}

			

			// add block to output
			return_data[bytes_encoded_virtual] = 251;
			bytes_encoded_virtual++;
			return_data[bytes_encoded_virtual] = ii;
			bytes_encoded_virtual++;
			return_data[bytes_encoded_virtual] = inbuf[i];
			bytes_encoded_virtual++;

			bytes_encoded += ii;
			ii--;
			i+= ii;
		}
		else {
			return_data[bytes_encoded_virtual] += inbuf[i];
			bytes_encoded++;
			bytes_encoded_virtual++;

		}
	}
	
	return return_data;
}

int rle_decode(unsigned char* outbuf,unsigned char* inbuf, int bytes_to_return, int inbuf_eof){
	// real bytes read
	int i = 0;
	int virtual_bytes_read = 0;

	for (i = 0; i < inbuf_eof && virtual_bytes_read < bytes_to_return; i++){
		if (inbuf[i] == 251 && i + 2 < inbuf_eof){
			for (int ii = 0; ii < inbuf[i+1]; ii++){
				outbuf[virtual_bytes_read] = inbuf[i+2];
				virtual_bytes_read++;	
			}
			i+= 2;

		}
		else {
			outbuf[virtual_bytes_read] = inbuf[i];
			virtual_bytes_read++;
		}
	}



	return virtual_bytes_read;
}
