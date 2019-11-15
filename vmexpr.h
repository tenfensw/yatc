//
// Yatc - Yet Another Tcl Clone
// An embeddable modern scripting language with Tcl-like syntax
// Copyright (C) Tim K/RoverAMD 2019 <timprogrammer@rambler.ru>.
// 
// vmexpr.c - Yatc's TinyExpr-based expression engine function
// declarations
//

#ifndef YATC_VMEXPR
#define YATC_VMEXPR

#include <math.h>
#include "tinyexpr/tinyexpr.h"
#include "cext.h"
#include "vmcommon.h"


int yatc_expressions_feedMath(const char* stringified, unsigned* ok);
unsigned yatc_expressions_conditionMet(const char* cond);

#endif