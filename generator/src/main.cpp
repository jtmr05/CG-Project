#include "primitive.hpp"

using std::string;


int main(int argc, const char** argv){

    string args[static_cast<size_t>(argc)];

    for(int i = 0; i < argc; i++){
        args[i].append(argv[i]);
    }

    const ErrorCode exit_code = primitive_writer(args, argc);
    handle_error(exit_code);

    return exit_code;
}