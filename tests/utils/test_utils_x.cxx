extern "C" {
  #include "test_utils_x.h"
}

#include <set>
#include <string>

struct agbtu_cxxstrset {
  std::set<std::string> v;
};

agbtu_cxxstrset * agbtu_cxxstrset_create(void) {
  return new agbtu_cxxstrset();
}

void agbtu_cxxstrset_add(agbtu_cxxstrset * self, const char * value) {
  self->v.insert(std::string(value));
}

int agbtu_cxxstrset_count(agbtu_cxxstrset * self) {
  return self->v.size();
}

int agbtu_cxxstrset_contains(agbtu_cxxstrset * self, const char * value) {
  return self->v.count(value)>0;
}

void agbtu_cxxstrset_destroy(agbtu_cxxstrset * self) {
  delete self;
}
