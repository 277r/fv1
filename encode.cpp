#include "encode.hpp"




framelist_entry encode_frame(FV1_HEADER data, AVFrame *current_frame, AVFrame *prev_frame){
	// probe frame to see what frame type is best

	// encode frame with that frametype
	return encode_c(data, current_frame, prev_frame);

}


int encode(char* infile, char* outfile){
	// use libav API
	StreamingContext *decoder = (StreamingContext*) calloc(1, sizeof(StreamingContext));
  	decoder->filename = infile;
	
	
	
	// open file
	decoder->avfc = avformat_alloc_context();
	if (decoder->avfc == 0){
		std::cout << "allocating avformat context failed\n";
		return -1;
	}
	if (avformat_open_input(&decoder->avfc, infile, NULL, NULL) != 0){
		std::cout << "opening input failed\n";
		return -1;
	}
	if (avformat_find_stream_info(decoder->avfc, NULL) < 0){
		std::cout << "getting stream info from stream failed\n";
		return -1;
	}

	
	
	// get first video stream
	// set pointer to zero so it can be checked later
	int video_stream_index;
	decoder->video_avs = 0;
	for (int i = 0; i < decoder->avfc->nb_streams; i++){
		if (decoder->avfc->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
			decoder->video_avs = decoder->avfc->streams[i];
			video_stream_index = i;
			// break out of for loop
			break;
		}
	}

	if (decoder->video_avs == 0){
		std::cout << "input file contains no video streams\n";
		return -1;
	}


	// find a suitable ffmpeg decoder, make context, copy parameters, open codec
	decoder->video_avc = avcodec_find_decoder(decoder->video_avs->codecpar->codec_id);
	if (decoder->video_avc == 0){
		std::cout << "could not find a decoder for this video\n";
		return -1;
	}
	
	decoder->video_avcc = avcodec_alloc_context3(decoder->video_avc);
	if (decoder->video_avcc == 0){
		std::cout << "failed to allocate memory for avcodec context\n";
		return -1;
	}

	if (avcodec_parameters_to_context(decoder->video_avcc, decoder->video_avs->codecpar) < 0){
		std::cout << "failed to copy parameters\n";
		return -1;
	}

	if (avcodec_open2(decoder->video_avcc, decoder->video_avc, NULL) < 0){
		std::cout << "failed to open codec\n";
		return -1;
	}



	// get video information needed for encoder
    AVRational input_framerate = av_guess_frame_rate(decoder->avfc, decoder->video_avs, NULL);
	FV1_HEADER j;
	j.fps_num = input_framerate.num;
	j.fps_den = input_framerate.den;
	j.px_x = decoder->video_avs->codec->width;
	j.px_y = decoder->video_avs->codec->height;
	j.pix_fmt = decoder->video_avs->codec->pix_fmt;
	j.colorspace = decoder->video_avs->codec->colorspace;



	// create framelist vector, that holds all the frames
	std::vector<framelist_entry> john;



	// allocate avframe, avpacket to get packets 
	AVFrame *input_frame = av_frame_alloc();
	if (input_frame == 0){
		std::cout << "could not allocate memory for frame\n";
		return -1;
	}

	AVPacket *input_packet = av_packet_alloc();
	if (input_packet == 0){
		std::cout << "could not allocate memory for packet\n";
		return -1;
	}

	AVFrame *previous_frame = 0;
	int res;
	// get frames one by one from framestream
	while (av_read_frame(decoder->avfc, input_packet) >= 0){
		// handle packet
		if (input_packet->stream_index == video_stream_index){		
			int res = avcodec_send_packet(decoder->video_avcc, input_packet);
			if (res < 0){
				std::cout << "error sending packet to decoder\n";
				// don't want any memory leaks
				av_packet_unref(input_packet);
				return -1;
			}

			// receive frames
			while (res >= 0){
				res = avcodec_receive_frame(decoder->video_avcc, input_frame);
				if (res == AVERROR(EAGAIN)) {
					break;
				}
				else if (res == AVERROR_EOF){
					
				}
				else if (res < 0){
					std::cout << "error receiving frame from encoder\n";
					return -1;
				}

				if (res >= 0){
					// encode frame, delete previous frame, set previous frame to this frame for next run
					john.push_back(encode_frame(j,input_frame, previous_frame));
					if (previous_frame != 0)
						av_frame_unref(previous_frame);
					previous_frame = input_frame;	
				}
		
			}
		
		}





	}
	// get last input packet after EOF signal
	av_read_frame(decoder->avfc, input_packet);
	if (input_packet->stream_index == video_stream_index){		
			int res = avcodec_send_packet(decoder->video_avcc, input_packet);
			if (res < 0){
				std::cout << "error sending packet to decoder\n";
				// don't want any memory leaks
				av_packet_unref(input_packet);
				return -1;
			}

			// receive frames
			while (res >= 0){
				res = avcodec_receive_frame(decoder->video_avcc, input_frame);
				if (res == AVERROR(EAGAIN)) {
					break;
				}
				else if (res == AVERROR_EOF){
					
				}
				else if (res < 0){
					std::cout << "error receiving frame from encoder\n";
					return -1;
				}

				if (res >= 0){
					// encode frame, delete previous frame, set previous frame to this frame for next run
					john.push_back(encode_frame(j,input_frame, previous_frame));
					if (previous_frame != 0)
						av_frame_unref(previous_frame);
					previous_frame = input_frame;	
				}
		
			}
		
		}
		
		// wipe packet after usage
		av_packet_unref(input_packet);
	
	
	if (previous_frame != 0)
		av_frame_unref(previous_frame);

	// turn the frame list into raw bytes (the framestream part, data part happens later)
	std::cout << john.size() << std::endl;
	unsigned long long framestream_creation_res;
	std::vector<unsigned long long> file_pointer_positions;
	unsigned char *output = create_frame_stream(john, framestream_creation_res, file_pointer_positions);
	std::cout << (void*)output << std::endl;
	std::cout << framestream_creation_res << std::endl;

	// finalize header
	j.frames[0] = 0;
	j.frames[1] = john.size();

	// write header and framestream
	std::ofstream fv1_file_out;
	fv1_file_out.open(outfile, std::ios::binary);

	fv1_file_out.write((const char*)&j, sizeof(FV1_HEADER));
	fv1_file_out.write((const char*)output, framestream_creation_res);
	// write datastream
	/*
	// append every frame's data buffer and set the frames 
	for (int i = 0; i < john.size(); i++){
		if (file_pointer_positions[i] != 0){
			// v = get current position
			unsigned long long v = fv1_file_out.tellp();
			// set v in framestream

			fv1_file_out.seekp(file_pointer_positions[i]);
			fv1_file_out.write((const char*)&v, sizeof(unsigned long long));
			// seek back to v
			fv1_file_out.seekp(v);
			// write frame
			fv1_file_out.write((const char*)john[i].location, john[i].size);
		}
	}
	*/
	// close file
	fv1_file_out.close();
}