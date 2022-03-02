#include "primitive.hpp"
#include "error_handler.hpp"


Primitive from_string(const string str){

    Primitive s = Primitive::__invalid;

    if(str == "plane")
        s = Primitive::plane;
    else if(str == "box")
        s = Primitive::box;
    else if(str == "sphere")
        s = Primitive::sphere;
    else if(str == "cone")
        s = Primitive::cone;

    return s;
}


//parse the arguments needed for each primitive and call the respective function
int primitive_switch(string args[], const int size){

    int exit_code = ErrorCode::default__;

    if(size > 1){

        Primitive primitive = from_string(args[1]);

        switch(primitive){

        case Primitive::plane:

            if(size < 5){
                exit_code = ErrorCode::not_enough_args;
            }
            else{

            }


            break;

        case Primitive::box:

            if(size < 5){
                exit_code = ErrorCode::not_enough_args;
            }
            else{

            }

            break;

        case Primitive::cone:

            if(size < 7){
                exit_code = ErrorCode::not_enough_args;
            }
            else{

            }

            break;

        case Primitive::sphere:

            if(size < 6){
                exit_code = ErrorCode::not_enough_args;
            }
            else{

            }

            break;

        default:
            exit_code = ErrorCode::invalid_argument;
            break;
        }
    }
    else
        exit_code = ErrorCode::not_enough_args;

    return exit_code;
}