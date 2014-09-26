#include <string>

namespace msfs {
	// Replaces all occurrences of "search" with "replace".
	void replace_substrs(const char *search,
						 size_t search_len,
						 const char *replace,
						 size_t replace_len,
						 std::string *s);
	
	// True iff s1 starts with s2.
	bool starts_with(const char *s1, const char *s2);
	
	// True iff s1 ends with s2.
	bool ends_with(const char *s1, const char *s2);
	
	// Remove leading and trailing whitespaces.
	std::string string_trim(const std::string& s);
	
	std::string string_prefix(const std::string& s, const char *sep = ".");
	std::string string_suffix(const std::string& s, const char *sep = ".");
}
