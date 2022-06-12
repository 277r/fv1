#include <vector>
#include "../bit_layout_reworked.hpp"




// extracted framestream block
struct fs_ex_block
{
	unsigned char t1, t2;
	unsigned long long p1, p2;
};

std::vector<fs_ex_block> extract_framestream(void *data, unsigned long long len);