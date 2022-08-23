

unsigned char* rle_encode(unsigned char*inbuf, int bytes_to_encode, int &bytes_encoded_virtual, int quality);
int rle_decode(unsigned char* outbuf,unsigned char* inbuf, int bytes_to_return, int inbuf_eof);