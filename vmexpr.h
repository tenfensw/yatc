#ifndef YATC_VMEXPR
#define YATC_VMEXPR

#include <math.h>
#include "tinyexpr/tinyexpr.h"
#include "cext.h"
#include "vmcommon.h"

// This one mostly wraps around TinyExpr

int yatc_expressions_feedMath(const char* stringified, unsigned* ok);
unsigned yatc_expressions_conditionMet(const char* cond);

#endif