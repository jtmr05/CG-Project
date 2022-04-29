#include "error_handler.hpp"

static void usage(){
    std::cerr << "Usage: \n" <<
        "\t engine <xml_file> [use_vbos:y|n] [dynamic_translate_tesselation_level]\n";
}

void handle_error(const ErrorCode e){

    if(e != ErrorCode::success)
        std::cerr << "engine: ";

    switch(e){

    case ErrorCode::invalid_argument:
        std::cerr << "Invalid argument found.\n";
        usage();
        break;

    case ErrorCode::io_error:
        std::cerr << "I/O error. Perhaps the specified path is incorrect?\n";
        break;

    case ErrorCode::not_enough_args:
        std::cerr << "Missing arguments.\n";
        usage();
        break;

    case ErrorCode::invalid_file_formatting:
        std::cerr << "Invalid xml formatting.\n";
        break;

    case ErrorCode::invalid_file_extension:
        std::cerr << "Invalid file extension.\n";
        break;

    default:
        break;
    }
}