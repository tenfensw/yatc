#ifndef YATC_VMEXPR
#define YATC_VMEXPR

#include <math.h>
#include "tinyexpr/tinyexpr.h"
#include "cext.h"

// This one mostly wraps around TinyExpr

int yatc_expressions_feedMath(const char* stringified, unsigned* ok);

#endif