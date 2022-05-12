#ifndef FILTERS_HPP
#define FILTERS_HPP

#include <string>
#include <regex>

int string_to_uint(const std::string &str);
double string_to_double(const std::string &str, double default_value = 0.0);
bool string_to_bool(const std::string &str, bool default_value = false);

bool has_3d_ext(const std::string &str);
bool has_xml_ext(const std::string &str);
bool has_patch_ext(const std::string &str);

std::string replace_extension(const std::string &fn, const std::string &ext);

#endif