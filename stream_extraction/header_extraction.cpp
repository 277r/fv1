#include "header_extraction.hpp"

// gives video data, returns qmatrixes 
void* extract_fv1_header_data(void *input, FV1_HEADER & rdata, unsigned long long &qmatsize){
	rdata = * (FV1_HEADER*)input;
	qmatsize = rdata.px_x;
	qmatsize *= rdata.px_y;
	// return the location of the quantization matrixes
	return ((input + sizeof(FV1_HEADER)));
}

void* extract_qmats(void* input, FV1_HEADER input_info, void *& qmat1, void *& qmat2, void *& qmat3){
	unsigned long long px_size = input_info.px_x * input_info.px_y;
	unsigned long long size1, size2, size3;
	if (input_info.pix_fmt == YUV420P888){
		size1 = px_size;
		// remember >> 2 is division by 4, is a a division by 2 on BOTH the x and y axis
		size2 = px_size >> 2;
		size3 = px_size >> 2;
	}
	// unsigned char because void isnt allowed
	qmat1 = new unsigned char[size1];
	qmat2 = new unsigned char[size2];
	qmat3 = new unsigned char[size3];

	// copy data (this should work)
	memcpy(qmat1, input, size1);
	memcpy(qmat2, input + size1, size2);
	memcpy(qmat3, input + size1 + size2, size3);



	// return location of video stream after q matrixes
	return (input + size1 + size2 + size3);
}