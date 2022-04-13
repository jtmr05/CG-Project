#include "error_handler.hpp"

static void usage(){
    std::cerr << "Usage: \n" <<
        "\t generator plane <length> <divisions> <output_file>\n" <<
        "\t generator box <units> <grid_size> <output_file>\n" <<
        "\t generator cone <radius> <height> <slices> <stacks> <output_file>\n" <<
        "\t generator sphere <radius> <slices> <stacks> <output_file>\n"  <<
        "\t generator torus <outter_radius> <inner_radius> <slices> <stacks> <output_file>\n";
}

void handle_error(const ErrorCode e){

    if(e != ErrorCode::success)
        std::cerr << "generator: ";

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

    case ErrorCode::invalid_file_extension:
        std::cerr << "Invalid file extension.\n";
        break;

    default:
        break;
    }
}