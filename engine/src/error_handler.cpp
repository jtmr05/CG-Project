#include "error_handler.hpp"

void usage(){
    std::cerr << "Usage: \n" <<
        "\t engine <xml_file>\n";
}

void handle_error(const ErrorCode e){

    switch(e){

    case ErrorCode::invalid_argument:
        std::cerr << "Invalid argument found.\n";
        usage();
        break;

    case ErrorCode::io_error:
        std::cerr << "I/O error. Perhaps path doesn't exist?\n";
        break;

    case ErrorCode::not_enough_args:
        std::cerr << "Missing arguments.\n";
        usage();
        break;

    case ErrorCode::invalid_xml_formatting:
        std::cerr << "Invalid xml formatting.\n";
        break;

    default:
        break;
    }
}