#ifndef ERROR_CODE_HPP
#define ERROR_CODE_HPP

#include <iostream>

enum ErrorCode {
    success,
    not_enough_args,
    invalid_argument,
    io_error,
    invalid_xml_formatting,
    invalid_file_extension,
};

void handle_error(const ErrorCode e);

#endif