#include <memory.h>
#include "../bit_layout_reworked.hpp"
void* extract_fv1_header_data(void *input, FV1_HEADER & rdata, unsigned long long &qmatsize);
void* extract_qmats(void* input, FV1_HEADER input_info, void *& qmat1, void *& qmat2, void *& qmat3);