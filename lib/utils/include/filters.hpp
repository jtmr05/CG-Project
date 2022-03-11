#ifndef FILTERS_HPP
#define FILTERS_HPP

#include <string>
#include <regex>

int string_to_uint(const std::string &str);
int string_to_int(const std::string &str, int default_value = 0);
bool has_3d_ext(const std::string &str);
bool has_xml_ext(const std::string &str);

#endif