//=====================================
// Extensions to basic clar functions
//=====================================
// This leaks on failure .. but who cares.
void cl_fail_v(const char * file, int line, const char * fmt, ...);
void clar__assert_equal_c(const char * file, int line, char* fmt, char c1,  char c2);

#define cl_assert_equal_c(c1,c2) clar__assert_equal_c(__FILE__, __LINE__, #c1 " != " #c2 " : %s != %s", c1, c2)

