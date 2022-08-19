// this file should not be included by anything other than main.cpp

#include <cstring>
#include "encode.hpp"
#include "decode.hpp"
#include <iostream>

#define HELPSTRING "run `fv1_transcode X input_file output_file N YYYYYY\nwhere X: either the letter d for decode, and e for encode\nN is optional, if more than 3 arguments are supplied, it is the video codec\nYYYYYY are other arguments that are not video io related\n\n"