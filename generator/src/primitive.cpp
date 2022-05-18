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



static constexpr size_t NUM_OF_PATCH_POINTS { 16 };

static constexpr int PLANE_ARGS  { 5 };
static constexpr int BOX_ARGS    { 5 };
static constexpr int CONE_ARGS   { 7 };
static constexpr int SPHERE_ARGS { 6 };
static constexpr int TORUS_ARGS  { 7 };
static constexpr int BEZIER_ARGS { 5 };



static Primitive from_string(const string &str){

    static const map<string, Primitive> str_primitives {
        { "plane",  Primitive::plane  },
        { "box",    Primitive::box    },
        { "sphere", Primitive::sphere },
        { "cone",   Primitive::cone   },
        { "torus",  Primitive::torus  },
        { "bezier", Primitive::bezier },
    };

    Primitive p { Primitive::__invalid };

    if(str_primitives.count(str) > 0)
        p = str_primitives.at(str);

    return p;
}



static inline CartPoint3d get_cubic_bezier_curve_point(const array<CartPoint3d, 4> &ctrl_points,
                                                       const array<double, 4> &binomial_coeffs){

    double x{}, y{}, z{};

    for(unsigned i{}; i < binomial_coeffs.size(); ++i){
        x += binomial_coeffs[i] * ctrl_points[ctrl_points.size() - 1 - i].x;
        y += binomial_coeffs[i] * ctrl_points[ctrl_points.size() - 1 - i].y;
        z += binomial_coeffs[i] * ctrl_points[ctrl_points.size() - 1 - i].z;
    }

    return { x, y, z };
}

static ErrorCode read_patch_file(const string &filename,
                                 vector<array<unsigned, NUM_OF_PATCH_POINTS>> &patch_indexes,
                                 vector<CartPoint3d> &ctrl_points){

    std::ifstream input_file{};
    input_file.open(filename, std::ios::in);

    if(!input_file.is_open())
        return ErrorCode::io_error;



    size_t num_of_patches {};

    if(!(input_file >> num_of_patches))
        return ErrorCode::invalid_file_formatting;

    patch_indexes.reserve(num_of_patches);



    size_t patch_count {};
    string line {};

    while(patch_count < num_of_patches && std::getline(input_file, line)){

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

        patch_indexes.push_back(std::move(elem));
        ++patch_count;
    }

    if(patch_count != num_of_patches)
        return ErrorCode::invalid_file_formatting;


    size_t num_of_ctrl_points {};

    if(!(input_file >> num_of_ctrl_points))
        return ErrorCode::invalid_file_formatting;

    ctrl_points.reserve(num_of_ctrl_points);

    CartPoint3d point {};

    for(unsigned i {}; i < num_of_ctrl_points && (input_file >> point); ++i)
        ctrl_points.push_back(point);

    if(ctrl_points.size() != num_of_ctrl_points)
        return ErrorCode::invalid_file_formatting;


    return ErrorCode::success;
}

static ErrorCode bezier_writer(const string &out_fn, const string &in_fn, unsigned tesselation_level){

    vector<array<unsigned, NUM_OF_PATCH_POINTS>> patch_indexes;
    vector<CartPoint3d> ctrl_points;

    const ErrorCode code { read_patch_file(in_fn, patch_indexes, ctrl_points) };
    if(code != ErrorCode::success)
        return code;

    /**
     * Input part is done, needed structures are created
     * Now it's time to treat them accordingly and take care of the output
     */

    std::ofstream vertexes_file{};
    vertexes_file.open(out_fn, std::ios::out | std::ios::trunc);

    std::ofstream normals_file{};
    normals_file.open(to_norm_extension(out_fn), std::ios::out | std::ios::trunc);

    if(!vertexes_file.is_open() || !normals_file.is_open())
        return ErrorCode::io_error;

    /* tesselation_level is incremented
     * in order for all the various patches
     * to be "stitched" together
     */
    const double time_step { 1.0 / static_cast<double>(tesselation_level++) };

    for(auto const& indexes_array : patch_indexes){

        vector<vector<CartPoint3d>> patch_matrix {};
        patch_matrix.reserve(tesselation_level);

        for(unsigned u{}; u < tesselation_level; ++u){

            vector<CartPoint3d> patch_row {};
            patch_row.reserve(tesselation_level);

            const double utime { time_step * static_cast<double>(u) };
            const double ucompl { 1.0 - utime };
            const array<double, 4> u_binomial_coeffs { //Bernstein polynomials definiton
                utime * utime * utime,
                3.0 * utime * utime * ucompl,
                3.0 * utime * ucompl * ucompl,
                1.0 * ucompl * ucompl * ucompl
            };

            auto iter { indexes_array.begin() };

            const CartPoint3d new_p0 {
                get_cubic_bezier_curve_point(
                    {
                        ctrl_points[*iter++],
                        ctrl_points[*iter++],
                        ctrl_points[*iter++],
                        ctrl_points[*iter++],
                    },
                    u_binomial_coeffs
                )
            };

            const CartPoint3d new_p1 {
                get_cubic_bezier_curve_point(
                    {
                        ctrl_points[*iter++],
                        ctrl_points[*iter++],
                        ctrl_points[*iter++],
                        ctrl_points[*iter++],
                    },
                    u_binomial_coeffs
                )
            };

            const CartPoint3d new_p2 {
                get_cubic_bezier_curve_point(
                    {
                        ctrl_points[*iter++],
                        ctrl_points[*iter++],
                        ctrl_points[*iter++],
                        ctrl_points[*iter++],
                    },
                    u_binomial_coeffs
                )
            };

            const CartPoint3d new_p3 {
                get_cubic_bezier_curve_point(
                    {
                        ctrl_points[*iter++],
                        ctrl_points[*iter++],
                        ctrl_points[*iter++],
                        ctrl_points[*iter],
                    },
                    u_binomial_coeffs
                )
            };

            for(unsigned v{}; v < tesselation_level; ++v){

                const double vtime { time_step * static_cast<double>(v) };
                const double vcompl { 1.0 - vtime };
                const array<double, 4> v_binomial_coeffs { //Bernstein polynomials definiton
                    vtime * vtime * vtime,
                    3.0 * vtime * vtime * vcompl,
                    3.0 * vtime * vcompl * vcompl,
                    1.0 * vcompl * vcompl * vcompl
                };

                const CartPoint3d pt {
                    get_cubic_bezier_curve_point(
                        { new_p0, new_p1, new_p2, new_p3 }, v_binomial_coeffs
                    )
                };

                patch_row.push_back(pt);
            }

            patch_matrix.push_back(std::move(patch_row));
        }

        for(unsigned i{}; i < static_cast<size_t>(tesselation_level - 1); ++i)
            for(unsigned j{}; j < static_cast<size_t>(tesselation_level - 1); ++j){

                const CartPoint3d normal1 {
                    plane_normal(
                        patch_matrix[i][j],
                        patch_matrix[i + 1][j],
                        patch_matrix[i][j + 1]
                    )
                };

                vertexes_file << patch_matrix[i][j]
                     << patch_matrix[i + 1][j]
                     << patch_matrix[i][j + 1];

                normals_file << normal1 << normal1 << normal1;

                const CartPoint3d normal2 {
                    plane_normal(
                        patch_matrix[i + 1][j + 1],
                        patch_matrix[i][j + 1],
                        patch_matrix[i + 1][j]
                    )
                };

                vertexes_file << patch_matrix[i + 1][j + 1]
                     << patch_matrix[i][j + 1]
                     << patch_matrix[i + 1][j];

                normals_file << normal2 << normal2 << normal2;
            }
    }

    return ErrorCode::success;
}

static ErrorCode torus_writer(const string &filename, int out_radius,
                              int in_radius, int slices, int stacks){

    std::ofstream vertexes_file{};
    vertexes_file.open(filename, std::ios::out | std::ios::trunc);

    std::ofstream normals_file{};
    normals_file.open(to_norm_extension(filename), std::ios::out | std::ios::trunc);

    if(!vertexes_file.is_open() || !normals_file.is_open())
        return ErrorCode::io_error;

    const angle_t zOx_delta { 360.0 / static_cast<angle_t>(slices) };
    const angle_t yOp_delta { 180.0 / static_cast<angle_t>(stacks) };
    const double diff_radius { static_cast<double>(out_radius - in_radius) / 2.0 };


    for(int sl{}; sl < slices; ++sl)

        for(int st{}; st < stacks; ++st){

            angle_t bottom_yOp { 90.0 - yOp_delta * static_cast<angle_t>(st) };
            angle_t top_yOp { 90.0 - yOp_delta * static_cast<angle_t>(st + 1) };
            angle_t bottom_zOx_offset {};
            angle_t top_zOx_offset {};

            /**
             * If yOp ever becomes negative, rotate zOx by 180
             * and negate yOp
             * That way the same point is represented
             * by this combo of angles
             */

            if(bottom_yOp < 0.0){
                bottom_yOp *= -1.0;
                bottom_zOx_offset = 180.0;
            }

            if(top_yOp < 0.0){
                top_yOp *= -1.0;
                top_zOx_offset = 180.0;
            }

            CartPoint3d bottom_starting_p {
                polar_to_cart( { diff_radius, bottom_zOx_offset, bottom_yOp } )
            };
            bottom_starting_p.z += static_cast<double>(in_radius) + diff_radius;

            CartPoint3d top_starting_p {
                polar_to_cart( { diff_radius, top_zOx_offset, top_yOp } )
            };
            top_starting_p.z += static_cast<double>(in_radius) + diff_radius;


            PolarPoint3d p1 {
                cart_to_polar(bottom_starting_p)
            };
            p1.zOx = static_cast<angle_t>(sl) * zOx_delta;

            PolarPoint3d p2 {
                cart_to_polar(bottom_starting_p)
            };
            p2.zOx = static_cast<angle_t>(sl + 1) * zOx_delta;

            PolarPoint3d p3 {
                cart_to_polar(top_starting_p)
            };
            p3.zOx = static_cast<angle_t>(sl) * zOx_delta;

            PolarPoint3d p4 {
                cart_to_polar(top_starting_p)
            };
            p4.zOx = static_cast<angle_t>(sl + 1) * zOx_delta;

            vertexes_file << p1 << p2 << p3;
            vertexes_file << p2 << p4 << p3;


            const PolarPoint3d normal1 { 1.0, p1.zOx + bottom_zOx_offset, bottom_yOp };
            const PolarPoint3d normal2 { 1.0, p2.zOx + bottom_zOx_offset, bottom_yOp };
            const PolarPoint3d normal3 { 1.0, p3.zOx + top_zOx_offset, top_yOp };
            const PolarPoint3d normal4 { 1.0, p4.zOx + top_zOx_offset, top_yOp };

            normals_file << normal1 << normal2 << normal3;
            normals_file << normal2 << normal4 << normal3;


            const PolarPoint3d neg_p1 { p1.radius, p1.zOx, 180 - p1.yOp };
            const PolarPoint3d neg_p2 { p2.radius, p2.zOx, 180 - p2.yOp };
            const PolarPoint3d neg_p3 { p3.radius, p3.zOx, 180 - p3.yOp };
            const PolarPoint3d neg_p4 { p4.radius, p4.zOx, 180 - p4.yOp };

            vertexes_file << neg_p1 << neg_p3 << neg_p2;
            vertexes_file << neg_p3 << neg_p4 << neg_p2;


            const PolarPoint3d neg_normal1 { 1.0, p1.zOx + bottom_zOx_offset, 180.0 - bottom_yOp };
            const PolarPoint3d neg_normal2 { 1.0, p2.zOx + bottom_zOx_offset, 180.0 - bottom_yOp };
            const PolarPoint3d neg_normal3 { 1.0, p3.zOx + top_zOx_offset, 180.0 - top_yOp};
            const PolarPoint3d neg_normal4 { 1.0, p4.zOx + top_zOx_offset, 180.0 - top_yOp};

            normals_file << neg_normal1 << neg_normal3 << neg_normal2;
            normals_file << neg_normal3 << neg_normal4 << neg_normal2;
        }

    return ErrorCode::success;
}

static ErrorCode sphere_writer(const string &filename, int radius, int slices, int stacks){

    std::ofstream vertexes_file{};
    vertexes_file.open(filename, std::ios::out | std::ios::trunc);

    std::ofstream normals_file{};
    normals_file.open(to_norm_extension(filename), std::ios::out | std::ios::trunc);

    if(!vertexes_file.is_open() || !normals_file.is_open())
        return ErrorCode::io_error;

    const angle_t zOx_delta { 360.0 / static_cast<angle_t>(slices) };
    const angle_t yOp_delta { 90.0 / static_cast<angle_t>(stacks / 2) };
    const double radius_d { static_cast<double>(radius) };


    for(int sl{}; sl < slices; ++sl){

        PolarPoint3d bp1 {
            radius_d,
            static_cast<angle_t>(sl) * zOx_delta,
            90.0
        };

        PolarPoint3d bp2 {
            radius_d,
            static_cast<angle_t>(sl + 1) * zOx_delta,
            90.0
        };

        for(int st{}; st < stacks / 2; ++st){

            const angle_t curr_yOp { 90.0 - yOp_delta * static_cast<angle_t>(st + 1) };

            const PolarPoint3d next_bp1 { radius_d, bp1.zOx, curr_yOp };
            const PolarPoint3d next_bp2 { radius_d, bp2.zOx, curr_yOp };

            vertexes_file << bp1 << bp2 << next_bp1;
            vertexes_file << next_bp1 << bp2 << next_bp2;


            normals_file << bp1.normalize() << bp2.normalize() << next_bp1.normalize();
            normals_file << next_bp1.normalize() << bp2.normalize() << next_bp2.normalize();


            const PolarPoint3d neg_bp1 { radius_d, bp1.zOx, 180.0 - bp1.yOp };
            const PolarPoint3d neg_bp2 { radius_d, bp2.zOx, 180.0 - bp2.yOp };
            const PolarPoint3d neg_next_bp1 { radius_d, next_bp1.zOx, 180.0 - next_bp1.yOp };
            const PolarPoint3d neg_next_bp2 { radius_d, next_bp2.zOx, 180.0 - next_bp2.yOp };

            vertexes_file << neg_next_bp1 << neg_next_bp2 << neg_bp1;
            vertexes_file << neg_bp1 << neg_next_bp2 << neg_bp2;


            normals_file << neg_next_bp1.normalize() << neg_next_bp2.normalize()
                         << neg_bp1.normalize();
            normals_file << neg_bp1.normalize() << neg_next_bp2.normalize()
                         << neg_bp2.normalize();


            bp1 = next_bp1;
            bp2 = next_bp2;
        }
    }

    return ErrorCode::success;
}

static ErrorCode cone_writer(const string &filename, int radius, int height, int slices, int stacks){

    std::ofstream vertexes_file{};
    vertexes_file.open(filename, std::ios::out | std::ios::trunc);

    std::ofstream normals_file{};
    normals_file.open(to_norm_extension(filename), std::ios::out | std::ios::trunc);

    if(!vertexes_file.is_open() || !normals_file.is_open())
        return ErrorCode::io_error;

    const angle_t zOx_delta { 360.0 / static_cast<angle_t>(slices) };
    const double height_delta { static_cast<double>(height) / static_cast<double>(stacks) };

    const CartPoint3d origin{};
    const CartPoint3d base_normal { 0.0, -1.0, 0.0 };

    const double normal_yOp {
        radian_to_degree(
            std::atan(
                static_cast<double>(height) / static_cast<double>(radius)
            )
        )
    };


    for(int sl{}; sl < slices; ++sl){

        PolarPoint3d bp1 {
            static_cast<double>(radius),
            static_cast<angle_t>(sl) * zOx_delta,
            90.0
        };

        PolarPoint3d bp2 {
            static_cast<double>(radius),
            static_cast<angle_t>(sl + 1) * zOx_delta,
            90.0
        };

        vertexes_file << bp1 << origin << bp2; //base

        normals_file << base_normal << base_normal << base_normal;


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
         *      r/(h*stacks) = r'/(h*(stacks - 1 - st)) <=> r/stacks = r'/(stacks - 1 - st)
         * since we start in the yOz plane, x is 0
         * that means z = stack_radius
         */

        for(int st{}; st < stacks; ++st){

            const double stack_radius {
                static_cast<double>((stacks - 1 - st) * radius) /
                static_cast<double>(stacks)
            };

            const double stack_height {
                static_cast<double>(st + 1) * height_delta
            };

            PolarPoint3d next_bp1 {
                cart_to_polar( { 0.0, stack_height, stack_radius } )
            };
            next_bp1.zOx = bp1.zOx;

            const PolarPoint3d next_bp2 { next_bp1.radius, bp2.zOx, next_bp1.yOp };

            vertexes_file << bp1 << bp2 << next_bp1;


            const PolarPoint3d normal1 { 1.0, bp1.zOx, normal_yOp };
            const PolarPoint3d normal2 { 1.0, bp2.zOx, normal_yOp };

            normals_file << normal1 << normal2 << normal1;

            if(st < stacks - 1){

                vertexes_file << next_bp1 << bp2 << next_bp2;

                normals_file << normal1 << normal2 << normal2;
            }


            bp1 = next_bp1;
            bp2 = next_bp2;
        }
    }

    return ErrorCode::success;
}

static ErrorCode box_writer(const string &filename, int units, int grid_size){

    std::ofstream vertexes_file{};
    vertexes_file.open(filename, std::ios::out | std::ios::trunc);

    std::ofstream normals_file{};
    normals_file.open(to_norm_extension(filename), std::ios::out | std::ios::trunc);

    if(!vertexes_file.is_open() || !normals_file.is_open())
        return ErrorCode::io_error;

    const double abs_max_coord { static_cast<double>(units) / 2.0 };
    const double step { static_cast<double>(units) / static_cast<double>(grid_size) };


    CartPoint3d p1 {}, p2 {}, p3 {}, p4 {};
    double x {}, y {}, z {};



    x = abs_max_coord;

    CartPoint3d normal1 { 0.0,  1.0, 0.0 };
    CartPoint3d normal2 { 0.0, -1.0, 0.0 };

    for(int i{}; i < grid_size; ++i, x -= step){

        z = abs_max_coord;

        for(int j{}; j < grid_size; ++j, z -= step){

            p1.y = p2.y = p3.y = p4.y = abs_max_coord;

            p1.x = x; p1.z = z;
            p2.x = x; p2.z = z - step;
            p3.x = x - step; p3.z = z;
            p4.x = x - step; p4.z = z - step;

            vertexes_file << p1 << p2 << p4;
            vertexes_file << p4 << p3 << p1;

            normals_file << normal1 << normal1 << normal1;
            normals_file << normal1 << normal1 << normal1;

            p1.y = p2.y = p3.y = p4.y = -abs_max_coord;

            vertexes_file << p1 << p3 << p2;
            vertexes_file << p2 << p3 << p4;

            normals_file << normal2 << normal2 << normal2;
            normals_file << normal2 << normal2 << normal2;
        }
    }

    y = abs_max_coord;

    normal1 = { 1.0,  0.0, 0.0 };
    normal2 = { -1.0, 0.0, 0.0 };

    for(int i{}; i < grid_size; ++i, y -= step){

        z = abs_max_coord;

        for(int j{}; j < grid_size; ++j, z -= step){

            p1.x = p2.x = p3.x = p4.x = abs_max_coord;

            p1.y = y; p1.z = z;
            p2.y = y; p2.z = z - step;
            p3.y = y - step; p3.z = z;
            p4.y = y - step; p4.z = z - step;

            vertexes_file << p1 << p4 << p2;
            vertexes_file << p1 << p3 << p4;

            normals_file << normal1 << normal1 << normal1;
            normals_file << normal1 << normal1 << normal1;

            p1.x = p2.x = p3.x = p4.x = -abs_max_coord;

            vertexes_file << p1 << p2 << p3;
            vertexes_file << p2 << p4 << p3;

            normals_file << normal2 << normal2 << normal2;
            normals_file << normal2 << normal2 << normal2;
        }
    }

    y = abs_max_coord;

    normal1 = { 0.0, 0.0, 1.0  };
    normal2 = { 0.0, 0.0, -1.0 };

    for(int i{}; i < grid_size; i++, y -= step){

        x = abs_max_coord;

        for(int j{}; j < grid_size; j++, x -= step){

            p1.z = p2.z = p3.z = p4.z = abs_max_coord;

            p1.y = y; p1.x = x;
            p2.y = y; p2.x = x - step;
            p3.y = y - step; p3.x = x;
            p4.y = y - step; p4.x = x - step;

            vertexes_file << p1 << p2 << p4;
            vertexes_file << p1 << p4 << p3;

            normals_file << normal1 << normal1 << normal1;
            normals_file << normal1 << normal1 << normal1;

            p1.z = p2.z = p3.z = p4.z = -abs_max_coord;

            vertexes_file << p1 << p3 << p2;
            vertexes_file << p2 << p3 << p4;

            normals_file << normal2 << normal2 << normal2;
            normals_file << normal2 << normal2 << normal2;
        }
    }

    return ErrorCode::success;
}

static ErrorCode plane_writer(const string &filename, int length, int divs){

    std::ofstream vertexes_file{};
    vertexes_file.open(filename, std::ios::out | std::ios::trunc);

    std::ofstream normals_file{};
    normals_file.open(to_norm_extension(filename), std::ios::out | std::ios::trunc);

    if(!vertexes_file.is_open() || !normals_file.is_open())
        return ErrorCode::io_error;

    const double abs_max_coord { static_cast<double>(length) / 2.0 };
    const double step { static_cast<double>(length) / static_cast<double>(divs) };

    CartPoint3d p1 {}, p2 {}, p3 {}, p4 {};
    const CartPoint3d normal { 0.0, 1.0, 0.0 };           //constant for all points of the plane

    double x { abs_max_coord };

    for(int i{}; i < divs; ++i, x -= step){

        double z { abs_max_coord };

        for(int j{}; j < divs; ++j, z -= step){

            p1.x = x; p1.z = z;
            p2.x = x; p2.z = z - step;
            p3.x = x - step; p3.z = z;
            p4.x = x - step; p4.z = z - step;

            vertexes_file << p1 << p4 << p3;
            vertexes_file << p1 << p2 << p4;

            normals_file << normal << normal << normal;
            normals_file << normal << normal << normal;
        }
    }

    return ErrorCode::success;
}



//parse the arguments needed for each primitive and call the respective function
ErrorCode primitive_writer(int size, const string args[]){

    unsigned args_index { 1 };

    if(size == 1)
        return ErrorCode::not_enough_args;



    const Primitive primitive { from_string(args[args_index++]) };

    switch(primitive){

    case Primitive::plane: {

        if(size < PLANE_ARGS)
            return ErrorCode::not_enough_args;

        const int length { string_to_uint(args[args_index++]) };
        const int divs { string_to_uint(args[args_index++]) };
        const string filename { args[args_index] };


        if(!has_3d_ext(filename))
            return ErrorCode::invalid_file_extension;

        if(length < 1 || divs < 1)
            return ErrorCode::invalid_argument;

        return plane_writer(filename, length, divs);
    }

    case Primitive::box: {

        if(size < BOX_ARGS)
            return ErrorCode::not_enough_args;

        const int units { string_to_uint(args[args_index++]) };
        const int grid_size { string_to_uint(args[args_index++]) };
        const string filename { args[args_index] };


        if(!has_3d_ext(filename))
            return ErrorCode::invalid_file_extension;

        if(units < 1 || grid_size < 1)
            return ErrorCode::invalid_argument;

        return box_writer(filename, units, grid_size);
    }

    case Primitive::cone: {

        if(size < CONE_ARGS)
            return ErrorCode::not_enough_args;

        const int radius { string_to_uint(args[args_index++]) };
        const int height { string_to_uint(args[args_index++]) };
        const int slices { string_to_uint(args[args_index++]) };
        const int stacks { string_to_uint(args[args_index++]) };
        const string filename { args[args_index] };


        if(!has_3d_ext(filename))
            return ErrorCode::invalid_file_extension;

        if(radius < 1 || height < 1 || slices < 3 || stacks < 1)
            return ErrorCode::invalid_argument;

        return cone_writer(filename, radius, height, slices, stacks);
    }

    case Primitive::sphere: {

        if(size < SPHERE_ARGS)
            return ErrorCode::not_enough_args;

        const int radius { string_to_uint(args[args_index++]) };
        const int slices { string_to_uint(args[args_index++]) };
        const int stacks { string_to_uint(args[args_index++]) };
        const string filename { args[args_index] };


        if(!has_3d_ext(filename))
            return ErrorCode::invalid_file_extension;

        if(radius < 1 || slices < 3 || stacks < 2 || stacks % 2 != 0)
            return ErrorCode::invalid_argument;

        return sphere_writer(filename, radius, slices, stacks);
    }

    case Primitive::torus: {

        if(size < TORUS_ARGS)
            return ErrorCode::not_enough_args;

        const int out_radius { string_to_uint(args[args_index++]) };
        const int in_radius { string_to_uint(args[args_index++]) };
        const int slices { string_to_uint(args[args_index++]) };
        const int stacks { string_to_uint(args[args_index++]) };
        const string filename { args[args_index] };


        if(!has_3d_ext(filename))
            return ErrorCode::invalid_file_extension;

        if(out_radius <  1 ||
           in_radius  <  0 ||
           stacks % 2 != 0 ||
           slices     <  3 ||
           stacks     <  2 ||
           out_radius <  in_radius
        )
            return ErrorCode::invalid_argument;

        return torus_writer(filename, out_radius, in_radius, slices, stacks);
    }

    case Primitive::bezier: {

        if(size < BEZIER_ARGS)
            return ErrorCode::not_enough_args;

        const string in_filename { args[args_index++] };
        const int tesselation_level { string_to_uint(args[args_index++]) };
        const string out_filename { args[args_index] };


        if(!has_patch_ext(in_filename) || !has_3d_ext(out_filename))
            return ErrorCode::invalid_file_extension;

        if(tesselation_level < 1)
            return ErrorCode::invalid_argument;

        return bezier_writer(out_filename, in_filename, static_cast<unsigned>(tesselation_level));
    }

    default:
        return ErrorCode::invalid_argument;
    }
}