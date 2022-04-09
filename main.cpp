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
	


}