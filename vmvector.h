#ifndef YATC_VECTOR
#define YATC_VECTOR

#include <stdlib.h>
#include <stdio.h>
#include "vmcommon.h"

struct YatcVector_s;
typedef struct YatcVector_s YatcVector;

YatcVector* yatc_vector_create(unsigned max);
unsigned yatc_vector_append(YatcVector* vc, YatcVariable* vl);
YatcVariable* yatc_vector_get(YatcVector* vc, unsigned index);
void yatc_vector_copy(YatcVector* v1, YatcVector* v2);
void yatc_vector_goodbye(YatcVector* vc);
unsigned yatc_vector_length(YatcVector* vc);

#endif