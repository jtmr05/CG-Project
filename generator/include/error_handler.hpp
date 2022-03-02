#ifndef ERROR_HANDLER_HPP
#define ERROR_HANDLER_HPP

typedef enum error_code {
    default__,
    not_enough_args,
    invalid_argument,
} ErrorCode;

void handler(ErrorCode e);

#endif