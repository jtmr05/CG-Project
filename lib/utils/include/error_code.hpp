#ifndef ERROR_CODE_HPP
#define ERROR_CODE_HPP

#include <iostream>

typedef enum error_code {
    default__,
    not_enough_args,
    invalid_argument,
    io_error,
} ErrorCode;

void handle_error(const ErrorCode e);

#endif