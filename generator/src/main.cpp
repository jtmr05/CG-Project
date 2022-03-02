#include "primitive.hpp"
#include "point.hpp"

using std::string;


int main(int argc, const char** argv){

    string args[static_cast<size_t>(argc)];

    for(int i = 0; i < argc; i++){
        args[i].append(argv[i]);
    }

    return primitive_writer(args, argc);
}