#ifndef ERROR_HANDLER_HPP
#define ERROR_HANDLER_HPP

#include <iostream>

typedef enum error_code {
    default__,
    not_enough_args,
    invalid_argument,
    io_error,
} ErrorCode;

void handler(const ErrorCode e);

#endif