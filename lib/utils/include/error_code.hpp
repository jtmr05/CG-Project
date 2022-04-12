#ifndef ERROR_CODE_HPP
#define ERROR_CODE_HPP

#include <iostream>

typedef enum error_code {
    success,
    not_enough_args,
    invalid_argument,
    io_error,
    invalid_xml_formatting,
    invalid_file_extension,
} ErrorCode;

void handle_error(const ErrorCode e);

#endif