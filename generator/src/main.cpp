#include "primitive.hpp"

using std::string;


int main(int argc, const char** argv){

    string args[static_cast<size_t>(argc)];

    for(int i = 0; i < argc; i++){
        args[i].append(argv[i]);
    }

    const int exit_code = primitive_writer(args, argc);
    ErrorCode err = static_cast<ErrorCode>(exit_code);
    handler(err);

    return exit_code;
}