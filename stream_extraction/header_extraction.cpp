#include "header_extraction.hpp"

// gives video data, returns qmatrixes 
void* extract_fv1_header_data(void *input, FV1_HEADER & rdata, long long &qmatsize){
	rdata = * (FV1_HEADER*)input;
	qmatsize = rdata.px_x;
	qmatsize *= rdata.px_y;
	// the size of the matrixes depends on the pixel format (the UV channels in YUV have 1/4th the size of the Y channel. 1 + 1/4 + 1/4 = 1.5)
	if (rdata.pix_fmt = px_fmts::YUV420P888){
		qmatsize *= 1.5;
	}
	return ((input + sizeof(FV1_HEADER)));
}

void* extract_qmats(void* input){
	
}