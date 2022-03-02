#include <iostream>
#include "error_handler.hpp"
#include "primitive.hpp"

using std::string;



int main(int argc, const char** argv){


    string args[static_cast<size_t>(argc)];

    for(int i = 0; i < argc; i++){
        args[i].append(argv[i]);
    }

    primitive_switch(args, argc);

    std::cout << "hello, world!\n";

    return 0;
}