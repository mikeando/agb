#ifndef AGB_TEST_UTILS_X_H
#define AGB_TEST_UTILS_X_H
/**
 * C wrappers for C++ functions/data structures, to make testing bareable.
 */

 typedef struct agbtu_cxxstrset agbtu_cxxstrset;

 agbtu_cxxstrset * agbtu_cxxstrset_create(void);
 void agbtu_cxxstrset_add(agbtu_cxxstrset * self, const char * value);
 int agbtu_cxxstrset_count(agbtu_cxxstrset * self);
 int agbtu_cxxstrset_contains(agbtu_cxxstrset * self, const char * value);
 void agbtu_cxxstrset_destroy(agbtu_cxxstrset * self);

 #endif
