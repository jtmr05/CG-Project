#include "primitive.hpp"

Primitive from_string(const string &str){

    Primitive s { Primitive::__invalid };

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

//matches with a non-negative integer
//-1 indicates no match found
int string_to_uint(const string &str){

    std::regex pattern { "^(\\+)?(\\d)+$", std::regex_constants::ECMAScript };

    if(std::regex_match(str, pattern))
        return stoi(str);
    else
        return -1;
}

// true if str ends in '.3d'
bool has_3d_ext(const string &str){

    std::regex pattern { "^(.+?)\\.3d$", std::regex_constants::ECMAScript };

    return std::regex_match(str, pattern);
}



int plane_writer(const string &filename, int length, int divs){

    int exit_code { ErrorCode::default__ };

    std::ofstream file{};
    file.open(filename, std::ios::out | std::ios::trunc);

    if(file.is_open()){

        const double abs_max_coord { static_cast<double>(length) / 2.0 };
        const double incr { static_cast<double>(length) / static_cast<double>(divs) };

        Point3d p1 {}, p2 {}, p3 {};


        double x { abs_max_coord };

        for(int i{}; i < divs; i++, x -= incr){

            double z { abs_max_coord };

            for(int j {}; j < divs; j++, z -= incr){

                p1.x = x; p1.z = z;
                p2.x = x; p2.z = z - incr;
                p3.x = x - incr; p3.z = z;
                file << p1 << p2 << p3;

                p1.x -= incr; p1.z -= incr;
                file << p2 << p1 << p3;
            }
        }
        file.close();
    }
    else
        exit_code = ErrorCode::io_error;


    return exit_code;
}


//parse the arguments needed for each primitive and call the respective function
int primitive_writer(const string args[], const int size){

    int exit_code { ErrorCode::default__ };
    int ind {1};

    if(size > 1){

        Primitive primitive = from_string(args[ind++]);

        switch(primitive){

        case Primitive::plane:

            if(size < PLANE_ARGS){
                exit_code = ErrorCode::not_enough_args;
            }
            else{
                const int length { string_to_uint(args[ind++]) };
                const int divs { string_to_uint(args[ind++]) };
                const string filename { args[ind] };

                if(length < 1 || divs < 1 || !has_3d_ext(filename))
                    exit_code = ErrorCode::invalid_argument;
                else
                    exit_code = plane_writer(filename, length, divs);

            }
            break;

        case Primitive::box:

            if(size < BOX_ARGS){
                exit_code = ErrorCode::not_enough_args;
            }
            else{
                const int units { string_to_uint(args[ind++]) };
                const int grid_side { string_to_uint(args[ind++]) };
                const string filename { args[ind] };

                if(units < 1 || grid_side < 1 || !has_3d_ext(filename))
                    exit_code = ErrorCode::invalid_argument;
                else{

                }
            }

            break;

        case Primitive::cone:

            if(size < CONE_ARGS){
                exit_code = ErrorCode::not_enough_args;
            }
            else{
                const int radius { string_to_uint(args[ind++]) };
                const int height { string_to_uint(args[ind++]) };
                const int slices { string_to_uint(args[ind++]) };
                const int stacks { string_to_uint(args[ind++]) };
                const string filename { args[ind] };

                if(radius < 1 || height < 1 || slices < 1 || stacks < 1 || !has_3d_ext(filename))
                    exit_code = ErrorCode::invalid_argument;
                else{

                }
            }

            break;

        case Primitive::sphere:

            if(size < SPHERE_ARGS){
                exit_code = ErrorCode::not_enough_args;
            }
            else{
                const int radius { string_to_uint(args[ind++]) };
                const int slices { string_to_uint(args[ind++]) };
                const int stacks { string_to_uint(args[ind++]) };
                const string filename { args[ind] };

                if(radius < 1 || slices < 1 || stacks < 1 || !has_3d_ext(filename))
                    exit_code = ErrorCode::invalid_argument;
                else{

                }
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