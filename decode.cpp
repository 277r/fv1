#include "decode.hpp"


int decode_frame(FV1_HEADER video_info, std::ifstream &f_in, unsigned long long frame_location_abs, AVFrame *frame_out, AVFrame *prev_frame, unsigned char frame_info){
	// can be optimized, array of functors
	if (frame_info == 1){
		decode_c(video_info, f_in, frame_location_abs, frame_out, prev_frame);
	}
}

// local function used to encode avpacket sent by libav
void encode_pkt(AVCodecContext* cc, AVFrame *recf, AVPacket *p, AVStream *s, AVFormatContext *avfc){
	int res = avcodec_send_frame(cc, recf);
	if (res < 0){

		std::cout << "error sending frame";
		std::cout << res << "\n";
		return;
	}

	while (res >= 0){
		res = avcodec_receive_packet(cc,p);
		if (res == AVERROR(EAGAIN) || res == AVERROR_EOF){
        	return;
		}
		else if (res < 0) {
			return;
		}

		av_packet_rescale_ts(p, cc->time_base, s->time_base);
        p->stream_index = s->index;
		
		res = av_interleaved_write_frame(avfc, p);

		if (res < 0){
			std::cout << "failed to write output packet\n";
			return;
		}

		av_packet_unref(p);
	}


}


int decode(char* infile, char* outfile, char *encname){
	
	int res;
	// create output context and check outfile
	
	char *end = strrchr(outfile, '.');
	if (end == 0){
		std::cout << "error: output file has no extension\n";
		return -1;
	}

	AVCodec *avc = NULL;
    AVCodecContext *avcc = NULL;
	AVPacket *pkt = NULL;

	AVOutputFormat *fmt;
    AVFormatContext *oc;
	AVStream *s;


	// keep avformat and delete ofstream below this block

    avformat_alloc_output_context2(&oc, NULL, NULL, outfile);
	if (oc <= 0){
		std::cout << "output format allocation failed\n";
		return -1;
	}
	fmt = oc->oformat;

	

	
	


	avc = avcodec_find_encoder_by_name(encname);
	if (avc < 0){
		std::cout << "could not find encoder with submitted codec name\n";
		return -1;
	}

	avcc = avcodec_alloc_context3(avc);
	if (avcc < 0){
		std::cout << "could not allocate avcc\n";
		return -1;
	}

	pkt = av_packet_alloc();
	if (pkt < 0){
		std::cout << "could not allocate avpacket\n";
		return -1;
	}

	// after all is initialized, initialize new avstream
	// add stream
	s = avformat_new_stream(oc, avc);
	if (s <= 0){
		std::cout << "allocating avstream failed\n";
		return -1;
	}
	s->id = oc->nb_streams - 1;

	// set video options
	//avcc->codec_id 


	// open input file and set options
	FV1_HEADER j;
	AVRational fps;
	std::ifstream input_fv1_file;
	input_fv1_file.open(infile, std::ios::binary);
	input_fv1_file.read((char*)&j, sizeof(FV1_HEADER));
	fps.num = j.fps_num;
	fps.den = j.fps_den;

	// set parameters

	avcc->bit_rate = 40 * 100 * 100;
    avcc->width = j.px_x;
    avcc->height = j.px_y;
    avcc->time_base = (AVRational){j.fps_den,j.fps_num};
	
	s->time_base = avcc->time_base;
    avcc->framerate = av_div_q({1,1},{(int)j.fps_num,(int)j.fps_den});

	// divide by fps to get right time
	s->duration = (j.frames[0] + 1);
	oc->duration = (j.frames[0] + 1);
	avcc->gop_size = 10;
    avcc->max_b_frames = 1;

	if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        avcc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	// stream should be added by now

	
    
	if (j.pix_fmt == AVPixelFormat::AV_PIX_FMT_YUV420P){
		// debug, remove later
		std::cout << "pixelformat setting worked :)\n";
		avcc->pix_fmt = AVPixelFormat::AV_PIX_FMT_YUV420P;
	}
	else {
		std::cout << "pixelformat setting failed\n";
		return -1;
	}
	
	// use the slow preset
	av_opt_set(avcc->priv_data, "preset", "slow", 0);

	res = avcodec_open2(avcc,avc, NULL);
	if (res < 0){
		std::cout << "could not open codec\n";
		return -1;
	}

	res = avcodec_parameters_from_context(s->codecpar, avcc);
    if (res < 0) {
		std::cout << "failed to copy stream parameters\n";
        return -1;
    }

	// before writing frames output information
    av_dump_format(oc, 0, outfile, 1);

	res = avio_open(&oc->pb, outfile, AVIO_FLAG_WRITE);
	if (res < 0) {
		std::cout << "failed open output file\n";
        return -1;
    }


	
	// write header and hope for best
	res = avformat_write_header(oc, NULL);
	if (res < 0){
		std::cout << "failed to write header\n";
		return -1;
	}


	// after setting all output options the input can be retrieved
	AVFrame *received_frame = av_frame_alloc();
	if (received_frame == 0){
		std::cout << "could not allocate memory for input frame\n";
		return -1;
	}
	AVFrame *prev_frame = av_frame_alloc();
	if (prev_frame == 0){
		std::cout << "could not allocate memory for prev frame\n";
		return -1;
	}

	received_frame->format = avcc->pix_fmt;
    received_frame->width  = avcc->width;
    received_frame->height = avcc->height;

    res = av_frame_get_buffer(received_frame, 0);
    if (res < 0) {
        std::cout << "could not allocate memory for buffers\n";
		return -1;
    }

	

	// update when 128 bit
	unsigned long long fs_size = 17 * ((j.frames[0] >> 1) + 1 + (j.frames[0] & 1));
	unsigned char *buf = new unsigned char[fs_size];
	input_fv1_file.read((char*)buf, fs_size);
	std::vector<fs_ex_block> frame_list = extract_framestream(buf,fs_size);

	// debug, but less verbose, remove later maybe
	std::cout << "fs size: " << fs_size << std::endl;	
	std::cout << "size: "<< frame_list.size() << std::endl;

	for (int i = 0; i < frame_list.size(); i++){
		res = av_frame_make_writable(received_frame);
		av_frame_make_writable(prev_frame);
	    if (res < 0){
			std::cout << "frame not writeable\n";
			return -1;
		}

		decode_frame(j, input_fv1_file, frame_list[i].p,received_frame, prev_frame, frame_list[i].t);
		received_frame->pts = i;
		
		// debug, remove later
		std::cout << "frame number: " << i << "\nframe id: " << frame_list[i].t << "\nframe_pos: " << frame_list[i].p << "\n\n"; 

		encode_pkt(avcc, received_frame, pkt, s, oc);

		av_frame_copy(prev_frame, received_frame);

	}
	encode_pkt(avcc, NULL, pkt, s, oc);

	// after writing video data, write trailer
	av_write_trailer(oc);
	
	// delete all resources
	avcodec_free_context(&avcc);
    av_frame_free(&prev_frame);
    av_frame_free(&received_frame);
    av_packet_free(&pkt);
    
	// close file
	avio_closep(&oc->pb);

	
		
}
	
