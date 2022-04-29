#include "primitive.hpp"

using std::string;
using std::vector;
using std::map;
using std::array;

enum Primitive {
    plane,
    box,
    sphere,
    cone,
    torus,
    bezier,
    __invalid,
};



static constexpr size_t NUM_OF_PATCH_POINTS {16};

static constexpr int PLANE_ARGS {5};
static constexpr int BOX_ARGS {5};
static constexpr int CONE_ARGS {7};
static constexpr int SPHERE_ARGS {6};
static constexpr int TORUS_ARGS {7};
static constexpr int BEZIER_ARGS {5};



static Primitive from_string(const string &str){

    static const map<string, Primitive> str_primitive_mapping {
        { "plane",  Primitive::plane },
        { "box",    Primitive::box },
        { "sphere", Primitive::sphere },
        { "cone", Primitive::cone },
        { "torus",  Primitive::torus },
        { "bezier",  Primitive::bezier },
    };

    Primitive p { Primitive::__invalid };

    if(str_primitive_mapping.count(str) > 0)
        p = str_primitive_mapping.at(str);

    return p;
}



static inline CartPoint3d cubic_bezier_curve_pt(const array<CartPoint3d, 4> &ctrl_points, double time){

    const double complement { 1.0 - time };

    const array<double, 4> binomial_coeffs { //Bernstein polynomials definiton
        time*time*time,
        3.0 * time*time * complement,
        3.0 * time * complement * complement,
        1.0 * complement * complement * complement
    };



    double x{}, y{}, z{};

    for(unsigned i{}; i < binomial_coeffs.size(); ++i){
        x += binomial_coeffs[i] * ctrl_points[ctrl_points.size() - 1 - i].x;
        y += binomial_coeffs[i] * ctrl_points[ctrl_points.size() - 1 - i].y;
        z += binomial_coeffs[i] * ctrl_points[ctrl_points.size() - 1 - i].z;
    }

    return { x, y, z };
}

static ErrorCode read_patch_file(
    const string &filename,
    vector<array<unsigned, NUM_OF_PATCH_POINTS>> &patch_indexes,
    vector<CartPoint3d> &ctrl_points){

        std::ifstream input_file{};
        input_file.open(filename, std::ios::in);

        if(!input_file.is_open())
            return ErrorCode::io_error;



        size_t patch_count {};

        if(!(input_file >> patch_count))
            return ErrorCode::invalid_file_formatting;

        patch_indexes.reserve(patch_count);



        unsigned i {};
        string line {};

        while(i < patch_count && std::getline(input_file, line)){

            if(line == "")
                continue;

            array<unsigned, NUM_OF_PATCH_POINTS> elem {};
            auto elem_iter { elem.begin() };

            size_t end{}, start{};

            do{
                end = line.find(',', start);

                const int index { string_to_uint(line.substr(start, end - start)) };
                if(index < 0)
                    return ErrorCode::invalid_file_formatting;

                *elem_iter = static_cast<unsigned>(index);
                ++elem_iter;

                if(end < std::string::npos)
                    start = end + 1;
            }
            while(end < std::string::npos && elem_iter != elem.end());

            if(elem_iter != elem.end())
                return ErrorCode::invalid_file_formatting;

            patch_indexes.push_back(elem);
            ++i;
        }


        size_t ctrl_point_count {};

        if(!(input_file >> ctrl_point_count))
            return ErrorCode::invalid_file_formatting;

        ctrl_points.reserve(ctrl_point_count);

        CartPoint3d point {};

        for(unsigned i {}; i < ctrl_point_count && (input_file >> point); ++i)
            ctrl_points.push_back(point);

        if(ctrl_points.size() != ctrl_point_count)
            return ErrorCode::invalid_file_formatting;



        input_file.close();

        return ErrorCode::success;
}

static ErrorCode bezier_writer(const string &out_fn, int tesselation_level, const string &in_fn){

    vector<array<unsigned, NUM_OF_PATCH_POINTS>> patch_indexes;
    vector<CartPoint3d> ctrl_points;

    const ErrorCode code { read_patch_file(in_fn, patch_indexes, ctrl_points) }; //runs fine
    if(code != ErrorCode::success)
        return code;

    /**
     * Input part is done, needed structures are created
     * Now it's time to treat them accordingly and take care of the output
     */

    std::ofstream file{};
    file.open(out_fn, std::ios::out | std::ios::trunc);

    if(!file.is_open())
        return ErrorCode::io_error;

    /* tesselation_level is incremented
     * in order for all the various patches
     * to be "stitched" together
     */
    const double time_step { 1.0 / static_cast<double>(tesselation_level++) };

    for(const auto& indexes_array : patch_indexes){

        vector<vector<CartPoint3d>> patch_matrix {};
        patch_matrix.reserve(static_cast<size_t>(tesselation_level));

        for(int u{}; u < tesselation_level; ++u){

            vector<CartPoint3d> patch_row {};
            patch_row.reserve(static_cast<size_t>(tesselation_level));

            auto iter { indexes_array.begin() };

            const CartPoint3d new_p0 {
                cubic_bezier_curve_pt(
                    {
                        ctrl_points[*iter++],
                        ctrl_points[*iter++],
                        ctrl_points[*iter++],
                        ctrl_points[*iter++],
                    },
                    time_step * static_cast<double>(u)
                )
            };

            const CartPoint3d new_p1 {
                cubic_bezier_curve_pt(
                    {
                        ctrl_points[*iter++],
                        ctrl_points[*iter++],
                        ctrl_points[*iter++],
                        ctrl_points[*iter++],
                    },
                    time_step * static_cast<double>(u)
                )
            };

            const CartPoint3d new_p2 {
                cubic_bezier_curve_pt(
                    {
                        ctrl_points[*iter++],
                        ctrl_points[*iter++],
                        ctrl_points[*iter++],
                        ctrl_points[*iter++],
                    },
                    time_step * static_cast<double>(u)
                )
            };

            const CartPoint3d new_p3 {
                cubic_bezier_curve_pt(
                    {
                        ctrl_points[*iter++],
                        ctrl_points[*iter++],
                        ctrl_points[*iter++],
                        ctrl_points[*iter],
                    },
                    time_step * static_cast<double>(u)
                )
            };

            for(int v{}; v < tesselation_level; ++v){
                const CartPoint3d pt {
                    cubic_bezier_curve_pt(
                        { new_p0, new_p1, new_p2, new_p3 }, time_step * static_cast<double>(v)
                    )
                };

                patch_row.push_back(pt);
            }

            patch_matrix.push_back(patch_row);
        }

        for(unsigned i{}; i < static_cast<size_t>(tesselation_level - 1); ++i)
            for(unsigned j{}; j < static_cast<size_t>(tesselation_level - 1); ++j){

                file << patch_matrix[i][j]
                     << patch_matrix[i + 1][j]
                     << patch_matrix[i][j + 1];

                file << patch_matrix[i + 1][j + 1]
                     << patch_matrix[i][j + 1]
                     << patch_matrix[i + 1][j];
            }
    }

    return ErrorCode::success;
}

static ErrorCode torus_writer(const string &filename, int out_radius,
                              int in_radius, int slices, int stacks){

    std::ofstream file{};
    file.open(filename, std::ios::out | std::ios::trunc);

    if(!file.is_open())
        return ErrorCode::io_error;

    const angle_t zOx_delta { 360.0 / static_cast<angle_t>(slices) };
    const angle_t yOp_delta { 180.0 / static_cast<angle_t>(stacks) };
    const double radius { static_cast<double>(out_radius - in_radius) / 2.0 };



    for(int i{}; i < slices; ++i){

        for(int j{}; j < stacks; ++j){

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

    return ErrorCode::success;
}

static ErrorCode sphere_writer(const string &filename, int radius, int slices, int stacks){

    std::ofstream file{};
    file.open(filename, std::ios::out | std::ios::trunc);

    if(!file.is_open())
        return ErrorCode::io_error;

    const angle_t zOx_delta { 360.0 / static_cast<angle_t>(slices) };
    const angle_t yOp_delta { 90.0 / static_cast<angle_t>(stacks / 2) };
    const double radius_d { static_cast<double>(radius) };


    for(int i{}; i < slices; ++i){

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

        for(int j{}; j < stacks/2; ++j){

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

    return ErrorCode::success;
}

static ErrorCode cone_writer(const string &filename, int radius, int height, int slices, int stacks){

    std::ofstream file{};
    file.open(filename, std::ios::out | std::ios::trunc);

    if(!file.is_open())
        return ErrorCode::io_error;

    const angle_t zOx_delta { 360.0 / static_cast<angle_t>(slices) };
    const double height_delta { static_cast<double>(height) / static_cast<double>(stacks) };

    const CartPoint3d origin{};

    for(int i{}; i < slices; ++i){

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

        for(int j{}; j < stacks; ++j){

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

    return ErrorCode::success;
}

static ErrorCode box_writer(const string &filename, int units, int grid_size){

    std::ofstream file{};
    file.open(filename, std::ios::out | std::ios::trunc);

    if(!file.is_open())
        return ErrorCode::io_error;

    const double abs_max_coord { static_cast<double>(units) / 2.0 };
    const double incr { static_cast<double>(units) / static_cast<double>(grid_size) };


    CartPoint3d p1 {}, p2 {}, p3 {}, p4 {};
    double x {}, y {}, z {};


    x = abs_max_coord;

    for(int i{}; i < grid_size; ++i, x -= incr){

        z = abs_max_coord;

        for(int j{}; j < grid_size; ++j, z -= incr){

            p1.y = p2.y = p3.y = p4.y = abs_max_coord;

            p1.x = x; p1.z = z;
            p2.x = x; p2.z = z - incr;
            p3.x = x - incr; p3.z = z;
            p4.x = x - incr; p4.z = z - incr;

            file << p1 << p2 << p4;
            file << p4 << p3 << p1;

            p1.y = p2.y = p3.y = p4.y = -abs_max_coord;

            file << p1 << p3 << p2;
            file << p2 << p3 << p4;
        }
    }

    y = abs_max_coord;

    for(int i{}; i < grid_size; ++i, y -= incr){

        z = abs_max_coord;

        for(int j{}; j < grid_size; ++j, z -= incr){

            p1.x = p2.x = p3.x = p4.x = abs_max_coord;

            p1.y = y; p1.z = z;
            p2.y = y; p2.z = z - incr;
            p3.y = y - incr; p3.z = z;
            p4.y = y - incr; p4.z = z - incr;

            file << p1 << p4 << p2;
            file << p1 << p3 << p4;

            p1.x = p2.x = p3.x = p4.x = -abs_max_coord;

            file << p1 << p2 << p3;
            file << p2 << p4 << p3;
        }
    }

    y = abs_max_coord;

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

            p1.z = p2.z = p3.z = p4.z = -abs_max_coord;

            file << p1 << p3 << p2;
            file << p2 << p3 << p4;
        }
    }

    file.close();

    return ErrorCode::success;
}

static ErrorCode plane_writer(const string &filename, int length, int divs){

    std::ofstream file{};
    file.open(filename, std::ios::out | std::ios::trunc);

    if(!file.is_open())
        return ErrorCode::io_error;

    const double abs_max_coord { static_cast<double>(length) / 2.0 };
    const double incr { static_cast<double>(length) / static_cast<double>(divs) };

    CartPoint3d p1 {}, p2 {}, p3 {}, p4 {};


    double x { abs_max_coord };

    for(int i{}; i < divs; ++i, x -= incr){

        double z { abs_max_coord };

        for(int j{}; j < divs; ++j, z -= incr){

            p1.x = x; p1.z = z;
            p2.x = x; p2.z = z - incr;
            p3.x = x - incr; p3.z = z;
            p4.x = x - incr; p4.z = z - incr;

            file << p1 << p4 << p3;
            file << p1 << p2 << p4;
        }
    }

    file.close();

    return ErrorCode::success;
}



//parse the arguments needed for each primitive and call the respective function
ErrorCode primitive_writer(const string args[], const int size){

    unsigned ind {1};

    if(size == 1)
        return ErrorCode::not_enough_args;



    const Primitive primitive { from_string(args[ind++]) };

    switch(primitive){

    case Primitive::plane: {

        if(size < PLANE_ARGS)
            return ErrorCode::not_enough_args;

        const int length { string_to_uint(args[ind++]) };
        const int divs { string_to_uint(args[ind++]) };
        const string filename { args[ind] };


        if(!has_3d_ext(filename))
            return ErrorCode::invalid_file_extension;

        if(length < 1 || divs < 1)
            return ErrorCode::invalid_argument;

        return plane_writer(filename, length, divs);
    }

    case Primitive::box: {

        if(size < BOX_ARGS)
            return ErrorCode::not_enough_args;

        const int units { string_to_uint(args[ind++]) };
        const int grid_size { string_to_uint(args[ind++]) };
        const string filename { args[ind] };


        if(!has_3d_ext(filename))
            return ErrorCode::invalid_file_extension;

        if(units < 1 || grid_size < 1)
            return ErrorCode::invalid_argument;

        return box_writer(filename, units, grid_size);
    }

    case Primitive::cone: {

        if(size < CONE_ARGS)
            return ErrorCode::not_enough_args;

        const int radius { string_to_uint(args[ind++]) };
        const int height { string_to_uint(args[ind++]) };
        const int slices { string_to_uint(args[ind++]) };
        const int stacks { string_to_uint(args[ind++]) };
        const string filename { args[ind] };


        if(!has_3d_ext(filename))
            return ErrorCode::invalid_file_extension;

        if(radius < 1 || height < 1 || slices < 3 || stacks < 1)
            return ErrorCode::invalid_argument;

        return cone_writer(filename, radius, height, slices, stacks);
    }

    case Primitive::sphere: {

        if(size < SPHERE_ARGS)
            return ErrorCode::not_enough_args;

        const int radius { string_to_uint(args[ind++]) };
        const int slices { string_to_uint(args[ind++]) };
        const int stacks { string_to_uint(args[ind++]) };
        const string filename { args[ind] };


        if(!has_3d_ext(filename))
            return ErrorCode::invalid_file_extension;

        if(radius < 1 || slices < 3 || stacks < 2 || stacks % 2 != 0)
            return ErrorCode::invalid_argument;

        return sphere_writer(filename, radius, slices, stacks);
    }

    case Primitive::torus: {

        if(size < TORUS_ARGS)
            return ErrorCode::not_enough_args;

        const int out_radius { string_to_uint(args[ind++]) };
        const int in_radius { string_to_uint(args[ind++]) };
        const int slices { string_to_uint(args[ind++]) };
        const int stacks { string_to_uint(args[ind++]) };
        const string filename { args[ind] };


        if(!has_3d_ext(filename))
            return ErrorCode::invalid_file_extension;

        if(
               out_radius <  1
            || in_radius  <  0
            || out_radius <  in_radius
            || slices     <  3
            || stacks     <  2
            || stacks % 2 != 0
        )
            return ErrorCode::invalid_argument;

        return torus_writer(filename, out_radius, in_radius, slices, stacks);
    }

    case Primitive::bezier: {

        if(size < BEZIER_ARGS)
            return ErrorCode::not_enough_args;

        const string input_filename { args[ind++] };
        const int tesselation_level { string_to_uint(args[ind++]) };
        const string output_filename { args[ind] };


        if(!has_patch_ext(input_filename) || !has_3d_ext(output_filename))
            return ErrorCode::invalid_file_extension;

        if(tesselation_level < 1)
            return ErrorCode::invalid_argument;

        return bezier_writer(output_filename, tesselation_level, input_filename);
    }

    default:
        return ErrorCode::invalid_argument;
    }
}