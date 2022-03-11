#include "filters.hpp"

using std::string;


//matches with and converts to a non-negative integer
//-1 indicates no match found
int string_to_uint(const string &str){

    std::regex pattern { "^(\\+)?(\\d)+$", std::regex_constants::ECMAScript };

    if(std::regex_match(str, pattern))
        return stoi(str);
    else
        return -1;
}

//matches with and a converts to an integer
//returns default_value in case no match was found
int string_to_int(const string &str, int default_value){

    std::regex pattern { "^(\\+|\\-)?(\\d)+$", std::regex_constants::ECMAScript };

    if(std::regex_match(str, pattern))
        return stoi(str);
    else
        return default_value;
}



// true if str ends in '.3d'
bool has_3d_ext(const string &str){

    std::regex pattern { "^(.+?)\\.3d$", std::regex_constants::ECMAScript };

    return std::regex_match(str, pattern);
}

// true if str ends in '.xml'
bool has_xml_ext(const string &str){

    std::regex pattern { "^(.+?)\\.xml$", std::regex_constants::ECMAScript };

    return std::regex_match(str, pattern);
}
