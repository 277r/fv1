#include "bit_layout_reworked.hpp"
#include "stream_extraction/header_extraction.hpp"


class FV1_DECODER {

public:
	void set_input_buf(void *data);
	void extract_header();




private:
	void *video_data;
	FV1_HEADER header_data;
	void *qmat1;
	void *qmat2;
	void *qmat3;


};