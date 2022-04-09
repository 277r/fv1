#include <vector>
#include "../bit_layout_reworked.hpp"


void *get_fs_location(void *input, unsigned long long qmat_size);


// extracted framestream block
struct fs_ex_block
{
	unsigned long long p1, p2;
	unsigned char t1, t2;
};

std::vector<fs_ex_block> extract_framestream(void *data, unsigned long long len);