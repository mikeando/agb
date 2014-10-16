#pragma once
#include "agb/internal/types.h"
//TODO: These should probably become varargs marcos that forward to 
//      functions so that we can capture file and line.
int agb__error_translate(AGBError * error, const char * message, int errcode);
int agb__error_from_string(AGBError * error, const char * message, int errcode, int git_errcode);
