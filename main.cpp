#include "main.hpp"







int main(int argc, char *argv[]){
	// handle args
	if (argc < 4){
		// help does not require a third argument, this is the only exception there currently is
		if (argc > 1 && strcmp(argv[1], "h") == 0){
			std::cout << HELPSTRING;
			return 0;
		}
		std::cout << "error: not enough arguments provided\nuse fv1_transcode h to show a list of arguments\n";
		return 0;
	}

	// input option definitions
	// true = convert from ffmpeg to fv1
	// false =convert from fv1 to ffmpeg
	bool convert_to_fv1;
	char *input_filename = argv[2];
	char *output_filename = argv[3];

	// get input options
	while (*(argv[1]) != 0){
		 if (*argv[1] == 'e'){
			// print day
			convert_to_fv1 = true;
		}
		else if (*argv[1] == 'd'){
			// 
			convert_to_fv1 = false;
		}
		*(argv[1])++;
	}

	std::cout << "input filename: " << input_filename << "\noutput filename: " << output_filename << "\ntranscoding to: " << (convert_to_fv1 ? std::string("fv1") : std::string("ffmpeg")); 
	if (convert_to_fv1){
		encode(input_filename, output_filename);
	}
	else {
		decode(input_filename, output_filename);
	}

}