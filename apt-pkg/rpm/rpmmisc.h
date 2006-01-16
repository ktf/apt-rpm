#ifndef RPMMISC_H
#define RPMMISC_H

#ifdef WITH_GNU_HASH_MAP

#include <ext/hash_map>

using namespace __gnu_cxx;

struct hash_string
{
   size_t operator()(string str) const {
      unsigned long h = 0; 
      const char *s = str.c_str();
      for (; *s; ++s)
	 h = 5*h + *s;
      return size_t(h);
   };
};

struct cstr_eq_pred
{
   size_t operator()(const char *s1, const char *s2) const
      { return strcmp(s1, s2) == 0; };
};
#endif /* WITH_GNU_HASH_MAP */

struct cstr_lt_pred
{
   size_t operator()(const char *s1, const char *s2) const
      { return strcmp(s1, s2) < 0; };
};

#endif

// vim:sts=3:sw=3
