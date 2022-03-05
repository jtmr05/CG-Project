#include "error_handler.hpp"

void usage(){
    std::cerr << "Usage: \n" ;/*<<
        "\t generator plane <length> <divisions> <output_file>\n" <<
        "\t generator box <units> <grid_size> <output_file>\n" <<
        "\t generator cone <radius> <height> <slices> <stacks> <output_file>\n" <<
        "\t generator sphere <radius> <slices> <stacks> <output_file>\n"; */
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

    default:
        break;
    }
}