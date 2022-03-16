#include "primitive.hpp"

using std::string;

typedef enum primitive {
    plane,
    box,
    sphere,
    cone,
    torus,
    __invalid,
} Primitive;

static const int PLANE_ARGS {5};
static const int BOX_ARGS {5};
static const int CONE_ARGS {7};
static const int SPHERE_ARGS {6};
static const int TORUS_ARGS {7};

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
    else if(str == "torus")
        s = Primitive::torus;

    return s;
}

ErrorCode torus_writer(const string &filename, int out_radius, int in_radius, int slices, int stacks){

    ErrorCode exit_code { ErrorCode::success };

    std::ofstream file{};
    file.open(filename, std::ios::out | std::ios::trunc);

    if(file.is_open()){

        const angle_t zOx_delta { 360.0 / static_cast<angle_t>(slices) };
        const angle_t yOp_delta { 180.0 / static_cast<angle_t>(stacks) };
        const double radius { static_cast<double>(out_radius - in_radius) / 2.0 };



        for(int i{}; i < slices; i++){

            for(int j{}; j < stacks; j++){

                angle_t bottom_yOp { 90.0 - (yOp_delta * static_cast<angle_t>(j)) };
                angle_t top_yOp { 90.0 - (yOp_delta * static_cast<angle_t>(j + 1)) };
                angle_t bottom_offset = 0.0;
                angle_t top_offset = 0.0;


                if(bottom_yOp < 0.0){
                    bottom_yOp *= -1.0;
                    bottom_offset = 180.0;
                }

                if(top_yOp < 0.0){
                    top_yOp *= -1.0;
                    top_offset = 180.0;
                }

                CartPoint3d out_bottom_start_p {
                    polar_to_cart( PolarPoint3d{ radius, bottom_offset, bottom_yOp } )
                };
                out_bottom_start_p.z += static_cast<double>(in_radius) + radius;

                CartPoint3d out_top_start_p {
                    polar_to_cart( PolarPoint3d{ radius, top_offset, top_yOp } )
                };
                out_top_start_p.z += static_cast<double>(in_radius) + radius;


                PolarPoint3d p1 {
                    cart_to_polar(out_bottom_start_p)
                };
                p1.zOx = static_cast<angle_t>(i) * zOx_delta;

                PolarPoint3d p2 {
                    cart_to_polar(out_bottom_start_p)
                };
                p2.zOx = static_cast<angle_t>(i+1) * zOx_delta;

                PolarPoint3d p3 {
                    cart_to_polar(out_top_start_p)
                };
                p3.zOx = static_cast<angle_t>(i) * zOx_delta;

                PolarPoint3d p4 {
                    cart_to_polar(out_top_start_p)
                };
                p4.zOx = static_cast<angle_t>(i+1) * zOx_delta;

                file << p1 << p2 << p3;
                file << p2 << p4 << p3;


                PolarPoint3d neg_p1 { p1 };
                PolarPoint3d neg_p2 { p2 };
                PolarPoint3d neg_p3 { p3 };
                PolarPoint3d neg_p4 { p4 };

                neg_p1.yOp = 180.0 - neg_p1.yOp;
                neg_p2.yOp = 180.0 - neg_p2.yOp;
                neg_p3.yOp = 180.0 - neg_p3.yOp;
                neg_p4.yOp = 180.0 - neg_p4.yOp;

                file << neg_p1 << neg_p3 << neg_p2;
                file << neg_p3 << neg_p4 << neg_p2;
            }
        }
        file.close();
    }
    else
        exit_code = ErrorCode::io_error;

    return exit_code;
}

ErrorCode sphere_writer(const string &filename, int radius, int slices, int stacks){

    ErrorCode exit_code { ErrorCode::success };

    std::ofstream file{};
    file.open(filename, std::ios::out | std::ios::trunc);

    if(file.is_open()){

        const angle_t zOx_delta { 360.0 / static_cast<angle_t>(slices) };
        const angle_t yOp_delta { 90.0 / static_cast<angle_t>(stacks / 2) };
        const double radius_d { static_cast<double>(radius) };


        for(int i{}; i < slices; i++){

            PolarPoint3d bp1 {
                radius_d,
                static_cast<angle_t>(i) * zOx_delta,
                90.0
            };

            PolarPoint3d bp2 {
                radius_d,
                static_cast<angle_t>(i + 1) * zOx_delta,
                90.0
            };

            for(int j{}; j < stacks/2; j++){

                const angle_t curr_yOp = 90.0 - (yOp_delta * static_cast<double>(j + 1));

                PolarPoint3d next_bp1 {
                    radius_d,
                    bp1.zOx,
                    curr_yOp
                };

                PolarPoint3d next_bp2 {
                    radius_d,
                    bp2.zOx,
                    curr_yOp
                };

                file << bp1 << bp2 << next_bp1;
                file << next_bp1 << bp2 << next_bp2;

                PolarPoint3d neg_bp1 { bp1 };
                PolarPoint3d neg_bp2 { bp2 };
                PolarPoint3d neg_next_bp1 { next_bp1 };
                PolarPoint3d neg_next_bp2 { next_bp2 };

                neg_bp1.yOp = 180.0 - neg_bp1.yOp;
                neg_bp2.yOp = 180.0 - neg_bp2.yOp;
                neg_next_bp1.yOp = 180.0 - neg_next_bp1.yOp;
                neg_next_bp2.yOp = 180.0 - neg_next_bp2.yOp;

                file << neg_next_bp1 << neg_next_bp2 << neg_bp1;
                file << neg_bp1 << neg_next_bp2 << neg_bp2;

                bp1 = next_bp1;
                bp2 = next_bp2;
            }
        }

        file.close();
    }
    else
        exit_code = ErrorCode::io_error;

    return exit_code;
}

ErrorCode cone_writer(const string &filename, int radius, int height, int slices, int stacks){

    ErrorCode exit_code { ErrorCode::success };

    std::ofstream file{};
    file.open(filename, std::ios::out | std::ios::trunc);

    if(file.is_open()){

        const angle_t zOx_delta { 360.0 / static_cast<angle_t>(slices) };
        const double height_delta { static_cast<double>(height) / static_cast<double>(stacks) };

        const CartPoint3d origin{};

        for(int i{}; i < slices; i++){

            PolarPoint3d bp1 {
                static_cast<double>(radius),
                static_cast<angle_t>(i) * zOx_delta,
                90.0
            };

            PolarPoint3d bp2 {
                static_cast<double>(radius),
                static_cast<angle_t>(i + 1) * zOx_delta,
                90.0
            };

            file << bp1 << origin << bp2; //base



            /**
             *    /| stack 4
             *   /_| stack 3
             *  /__| stack 2
             * /___| stack 1
             *
             *
             * height is stacks * height_delta; base length is radius
             * on each stack level, the height is given by height - (stack_level * height_delta)
             * thus, we can calculate the radius for that given stack using the similar triangles
             * formula:
             *      r/(h*stacks) = r'/(h*(stacks - 1 - j)) <=> r/stacks = r'/(stacks - 1 - j)
             * since we start in the yOz plane, x is 0
             * that means z = stack_radius
             */

            for(int j{}; j < stacks; j++){

                const double radius__ {
                    static_cast<double>((stacks - 1 - j) * radius) /
                    static_cast<double>(stacks)
                };

                PolarPoint3d next_bp1 {
                    cart_to_polar( { 0.0, static_cast<double>(j + 1) * height_delta, radius__ } )
                };
                next_bp1.zOx = bp1.zOx;

                const PolarPoint3d next_bp2 {
                    next_bp1.radius, next_bp1.zOx + zOx_delta, next_bp1.yOp
                };

                file << bp1 << bp2 << next_bp1;
                if(j < stacks - 1)
                    file << next_bp1 << bp2 << next_bp2;

                bp1 = next_bp1;
                bp2 = next_bp2;
            }
        }

        file.close();
    }
    else
        exit_code = ErrorCode::io_error;

    return exit_code;
}

ErrorCode box_writer(const string &filename, int units, int grid_size){

    ErrorCode exit_code { ErrorCode::success };

    std::ofstream file{};
    file.open(filename, std::ios::out | std::ios::trunc);

    if(file.is_open()){

        const double abs_max_coord { static_cast<double>(units) / 2.0 };
        const double incr { static_cast<double>(units) / static_cast<double>(grid_size) };


        CartPoint3d p1 {}, p2 {}, p3 {}, p4 {};
        double x {}, y {}, z {};


        x = abs_max_coord;

        for(int i{}; i < grid_size; i++, x -= incr){

            z = abs_max_coord;

            for(int j{}; j < grid_size; j++, z -= incr){

                p1.y = p2.y = p3.y = p4.y = 0.0;

                p1.x = x; p1.z = z;
                p2.x = x; p2.z = z - incr;
                p3.x = x - incr; p3.z = z;
                p4.x = x - incr; p4.z = z - incr;

                file << p1 << p3 << p2;
                file << p2 << p3 << p4;

                p1.y = p2.y = p3.y = p4.y = abs_max_coord * 2.0;

                file << p1 << p2 << p4;
                file << p4 << p3 << p1;
            }
        }

        y = abs_max_coord * 2.0;

        for(int i{}; i < grid_size; i++, y -= incr){

            z = abs_max_coord;

            for(int j{}; j < grid_size; j++, z -= incr){

                p1.x = p2.x = p3.x = p4.x = abs_max_coord;

                p1.y = y; p1.z = z;
                p2.y = y; p2.z = z - incr;
                p3.y = y - incr; p3.z = z;
                p4.y = y - incr; p4.z = z - incr;

                file << p1 << p4 << p2;
                file << p1 << p3 << p4;

                p1.x = p2.x = p3.x = p4.x = - abs_max_coord;

                file << p1 << p2 << p3;
                file << p2 << p4 << p3;
            }
        }

        y = abs_max_coord * 2.0;

        for(int i{}; i < grid_size; i++, y -= incr){

            x = abs_max_coord;

            for(int j{}; j < grid_size; j++, x -= incr){

                p1.z = p2.z = p3.z = p4.z = abs_max_coord;

                p1.y = y; p1.x = x;
                p2.y = y; p2.x = x - incr;
                p3.y = y - incr; p3.x = x;
                p4.y = y - incr; p4.x = x - incr;

                file << p1 << p2 << p4;
                file << p1 << p4 << p3;

                p1.z = p2.z = p3.z = p4.z = - abs_max_coord;

                file << p1 << p3 << p2;
                file << p2 << p3 << p4;
            }
        }

        file.close();
    }
    else
        exit_code = ErrorCode::io_error;

    return exit_code;
}

ErrorCode plane_writer(const string &filename, int length, int divs){

    ErrorCode exit_code { ErrorCode::success };

    std::ofstream file{};
    file.open(filename, std::ios::out | std::ios::trunc);

    if(file.is_open()){

        const double abs_max_coord { static_cast<double>(length) / 2.0 };
        const double incr { static_cast<double>(length) / static_cast<double>(divs) };

        CartPoint3d p1 {}, p2 {}, p3 {}, p4 {};


        double x { abs_max_coord };

        for(int i{}; i < divs; i++, x -= incr){

            double z { abs_max_coord };

            for(int j{}; j < divs; j++, z -= incr){

                p1.x = x; p1.z = z;
                p2.x = x; p2.z = z - incr;
                p3.x = x - incr; p3.z = z;
                p4.x = x - incr; p4.z = z - incr;

                file << p1 << p2 << p3;
                file << p2 << p4 << p3;
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

    ErrorCode exit_code { ErrorCode::success };
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
                const int grid_size { string_to_uint(args[ind++]) };
                const string filename { args[ind] };

                if(units < 1 || grid_size < 1 || !has_3d_ext(filename))
                    exit_code = ErrorCode::invalid_argument;
                else
                    exit_code = box_writer(filename, units, grid_size);
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
                if(radius < 1 || slices < 3 || stacks < 2 || stacks % 2 != 0 || !has_3d_ext(filename))
                    exit_code = ErrorCode::invalid_argument;
                else
                    exit_code = sphere_writer(filename, radius, slices, stacks);
            }

            break;

        case Primitive::torus:

            if(size < TORUS_ARGS)
                exit_code = ErrorCode::not_enough_args;
            else{
                const int out_radius { string_to_uint(args[ind++]) };
                const int in_radius { string_to_uint(args[ind++]) };
                const int slices { string_to_uint(args[ind++]) };
                const int stacks { string_to_uint(args[ind++]) };
                const string filename { args[ind] };

                if(out_radius < 1 || in_radius < 0 || out_radius < in_radius ||
                    slices < 3 || stacks < 2 || stacks % 2 != 0 || !has_3d_ext(filename))
                    exit_code = ErrorCode::invalid_argument;
                else
                    exit_code = torus_writer(filename, out_radius, in_radius, slices, stacks);
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