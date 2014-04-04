#include "utils/clarx.h"
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include "clar/clar.h"

//=====================================
// Extensions to basic clar functions
//=====================================
// This leaks on failure .. but who cares.
void cl_fail_v(const char * file, int line, const char * fmt, ...) {
	va_list ap;
	va_start(ap,fmt);
	char * mesg;
	vasprintf(&mesg,fmt,ap);
	va_end(ap);
	clar__fail(file,line,"Test Failed",mesg,1);
}

void clar__assert_equal_c(const char * file, int line, char* fmt, char c1, char c2){
	char c1s[5];
	char c2s[5];
	if(isprint(c1)) {
		snprintf(c1s,5,"'%c'",c1);
	} else {
		snprintf(c1s,5,"0x%02x",(int)(unsigned char)c1);
	}
	if(isprint(c2)) {
		snprintf(c2s,5,"'%c'",c2);
	} else {
		snprintf(c2s,5,"0x%02x",(int)(unsigned char)c2);
	}
	if(c1!=c2) {
		cl_fail_v(file,line,fmt,c1s,c2s);
	}
}

