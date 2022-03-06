#include "primitive.hpp"

typedef enum primitive {
    plane,
    box,
    sphere,
    cone,
    __invalid,
} Primitive;

const int PLANE_ARGS {5};
const int BOX_ARGS {5};
const int CONE_ARGS {7};
const int SPHERE_ARGS {6};

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

//matches with and converts a non-negative integer
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



ErrorCode sphere_writer(const string &filename, int radius, int slices, int stacks){

    ErrorCode exit_code { ErrorCode::default__ };

    return exit_code;
}

ErrorCode cone_writer(const string &filename, int radius, int height, int slices, int stacks){

    ErrorCode exit_code { ErrorCode::default__ };

    std::ofstream file{};
    file.open(filename, std::ios::out | std::ios::trunc);

    if(file.is_open()){

        const angle_t angle_delta { 360.0 / static_cast<angle_t>(slices) };
        const double height_delta { static_cast<double>(height) / static_cast<double>(stacks) };

        const CartPoint3d origin{};
        const CartPoint3d top_vertex { 0.0, static_cast<double>(height), 0.0 };

        for(int i{}; i < slices; i++){

            PolarPoint3d bp1 {
                static_cast<double>(radius),
                static_cast<angle_t>(i) * angle_delta,
                90.0
            };

            PolarPoint3d bp2 {
                static_cast<double>(radius),
                static_cast<angle_t>(i + 1) * angle_delta,
                90.0
            }; //potential bug

            file << bp1 << origin << bp2; //base



            /**
             *    /| stack 4
             *   /_| stack 3
             *  /__| stack 2
             * /___| stack 1
             *
             * height is h; base length is radius
             * on each stack level, the height is given by height - (stack_level * height_delta)
             * thus, we can calculate the radius for that given stack using the similar triangles
             * formula
             * since we start in the yOz plane, x is 0
             * that means z = stack_radius
             */

            for(int j{}; j < stacks; j++){

                if(j == stacks - 1)
                    file << bp1 << bp2 << top_vertex;
                else{
                    const double new_radius {
                        static_cast<double>((height - (j + 1) * height_delta) * radius) /
                        static_cast<double>(height)
                    };

                    CartPoint3d next_bp1 { 0.0, (j + 1) * height_delta, new_radius };
                    PolarPoint3d next_bp2 { cart_to_polar(next_bp1) };
                    next_bp2.zOx += angle_delta;

                    file << bp1 << next_bp1 << bp2;
                    file << next_bp1 << next_bp2 << bp2;

                    bp1 = cart_to_polar(next_bp1);
                    bp2 = next_bp2;
                }
            }
        }

        file.close();
    }
    else
        exit_code = ErrorCode::io_error;

    return exit_code;
}

ErrorCode box_writer(const string &filename, int units, int grid_side){

    ErrorCode exit_code { ErrorCode::default__ };

    return exit_code;
}

ErrorCode plane_writer(const string &filename, int length, int divs){

    ErrorCode exit_code { ErrorCode::default__ };

    std::ofstream file{};
    file.open(filename, std::ios::out | std::ios::trunc);

    if(file.is_open()){

        const double abs_max_coord { static_cast<double>(length) / 2.0 };
        const double incr { static_cast<double>(length) / static_cast<double>(divs) };

        CartPoint3d p1 {}, p2 {}, p3 {};


        double x { abs_max_coord };

        for(int i{}; i < divs; i++, x -= incr){

            double z { abs_max_coord };

            for(int j{}; j < divs; j++, z -= incr){

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
ErrorCode primitive_writer(const string args[], const int size){

    ErrorCode exit_code { ErrorCode::default__ };
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

            if(size < BOX_ARGS)
                exit_code = ErrorCode::not_enough_args;
            else{
                const int units { string_to_uint(args[ind++]) };
                const int grid_side { string_to_uint(args[ind++]) };
                const string filename { args[ind] };

                if(units < 1 || grid_side < 1 || !has_3d_ext(filename))
                    exit_code = ErrorCode::invalid_argument;
                else
                    exit_code = box_writer(filename, units, grid_side);
            }

            break;

        case Primitive::cone:

            if(size < CONE_ARGS)
                exit_code = ErrorCode::not_enough_args;
            else{
                const int radius { string_to_uint(args[ind++]) };
                const int height { string_to_uint(args[ind++]) };
                const int slices { string_to_uint(args[ind++]) };
                const int stacks { string_to_uint(args[ind++]) };
                const string filename { args[ind] };

                if(radius < 1 || height < 1 || slices < 3 || stacks < 1 || !has_3d_ext(filename))
                    exit_code = ErrorCode::invalid_argument;
                else
                    exit_code = cone_writer(filename, radius, height, slices, stacks);
            }

            break;

        case Primitive::sphere:

            if(size < SPHERE_ARGS)
                exit_code = ErrorCode::not_enough_args;
            else{
                const int radius { string_to_uint(args[ind++]) };
                const int slices { string_to_uint(args[ind++]) };
                const int stacks { string_to_uint(args[ind++]) };
                const string filename { args[ind] };

                //can stacks be less than 1 for sphere
                if(radius < 1 || slices < 3 || stacks < 1 || !has_3d_ext(filename))
                    exit_code = ErrorCode::invalid_argument;
                else
                    exit_code = sphere_writer(filename, radius, slices, stacks);
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