#include "StringUtils.h"
#include <string.h>

namespace msfs {

void replace_substrs(const char *search,
                     size_t search_len,
                     const char *replace,
                     size_t replace_len,
                     std::string *s) {
  size_t pos = 0;
  while ((pos = s->find(search, pos, search_len)) != std::string::npos) {
    s->replace(pos, search_len, replace, replace_len);
    pos += replace_len;
  }
}

bool starts_with(const char *s1, const char *s2) {
  return strncmp(s1, s2, strlen(s2)) == 0;
}

bool ends_with(const char *s1, const char *s2) {
  size_t s1_length = strlen(s1);
  size_t s2_length = strlen(s2);

  if (s2_length > s1_length) {
    return false;
  }

  const char* start = s1 + (s1_length - s2_length);
  return strncmp(start, s2, s2_length) == 0;
}

static const char kWhitespace[] = " \n\r\t";

std::string string_trim(const std::string& s) {
  std::string::size_type first = s.find_first_not_of(kWhitespace);
  std::string::size_type last  = s.find_last_not_of(kWhitespace);

  if (first == std::string::npos || last == std::string::npos) {
    return std::string("");
  }

  return s.substr(first, last - first + 1);
}

std::string string_prefix(const std::string& s, const char *sep) {
	std::string::size_type first = s.find_first_of(sep);
	if (first != std::string::npos)
		return s.substr(0, first);
	else
		return std::string("");
}


std::string string_suffix(const std::string& s, const char *sep) {
	std::string::size_type last = s.find_last_of(sep);
	if (last != std::string::npos)
		return s.substr(last + 1);
	else
		return std::string("");
}

}

