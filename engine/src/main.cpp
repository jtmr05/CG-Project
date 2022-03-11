#include "display.hpp"



int main(int argc, char **argv){

    const ErrorCode exit_code = start(argc, argv);
    handle_error(exit_code);

    return exit_code;
}