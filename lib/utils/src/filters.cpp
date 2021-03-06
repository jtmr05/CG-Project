#include "filters.hpp"

using std::string;


//matches with and converts to a non-negative integer
//-1 indicates no match found
int string_to_uint(const string &str){

    const std::regex pattern { "^(\\s)*(\\+)?(\\d)+(\\s)*$", std::regex_constants::ECMAScript };

    if(std::regex_match(str, pattern))
        return std::stoi(str);

    return -1;
}

//matches C-string with and converts to a double
//returns default_value in case no match was found
double string_to_double(const string &str, double default_value){

    const std::regex pattern {
        "^(\\s)*(\\+|\\-)?(\\d)+(\\.(\\d)+)?(\\s)*$",
        std::regex_constants::ECMAScript
    };

    if(std::regex_match(str, pattern))
        return std::stod(str);

    return default_value;
}

//matches C-string with and converts to a bool
//returns default_value in case no match was found
bool string_to_bool(const std::string &str, bool default_value){

    const std::regex true_pattern {
        "^(\\s)*[tT][rR][uU][eE](\\s)*$",
        std::regex_constants::ECMAScript
    };
    const std::regex false_pattern {
        "^(\\s)*[fF][aA][lL][sS][eE](\\s)*$",
        std::regex_constants::ECMAScript
    };

    if(std::regex_match(str, true_pattern))
        return true;
    else if(std::regex_match(str, false_pattern))
        return false;

    return default_value;
}



// true if str ends in '.3d'
bool has_3d_ext(const string &str){

    const std::regex pattern { "^(.+?)\\.3d$", std::regex_constants::ECMAScript };

    return std::regex_match(str, pattern);
}

// true if str ends in '.xml'
bool has_xml_ext(const string &str){

    const std::regex pattern { "^(.+?)\\.xml$", std::regex_constants::ECMAScript };

    return std::regex_match(str, pattern);
}

// true if str ends in '.patch'
bool has_patch_ext(const string &str){

    const std::regex pattern { "^(.+?)\\.patch$", std::regex_constants::ECMAScript };

    return std::regex_match(str, pattern);
}



// prefixing a dot in ext is optional
static string replace_extension(const string &str, const string &ext){

    const size_t last_dot_pos { str.find_last_of('.') };
    const string raw_filename {
        (last_dot_pos < string::npos)
            ? std::move(str.substr(0, last_dot_pos))
            : str
    };

    if(ext.at(0) != '.')
        return raw_filename + '.' + ext;

    return raw_filename + ext;
}

string to_norm_extension(const string &str){
    return replace_extension(str, ".norm");
}

string to_text_extension(const string &str){
    return replace_extension(str, ".text");
}
