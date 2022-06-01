#include "primitive.hpp"

using std::string;
using std::vector;
using std::map;
using std::array;
using std::tuple;

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



static
tuple<ErrorCode, vector<array<unsigned, NUM_OF_PATCH_POINTS>>, vector<CartPoint3d>>
read_patch_file(const string &filename){

    std::ifstream input_file{};
    input_file.open(filename, std::ios::in);

    vector<array<unsigned, NUM_OF_PATCH_POINTS>> patch_indexes {};
    vector<CartPoint3d> ctrl_points {};

    if(!input_file.is_open())
        return {
            ErrorCode::io_error,
            std::move(patch_indexes),
            std::move(ctrl_points)
        };



    size_t num_of_patches {};

    if(!(input_file >> num_of_patches))
        return {
            ErrorCode::invalid_file_formatting,
            std::move(patch_indexes),
            std::move(ctrl_points)
        };

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
                return {
                    ErrorCode::invalid_file_formatting,
                    std::move(patch_indexes),
                    std::move(ctrl_points)
                };

            *elem_iter = static_cast<unsigned>(index);
            ++elem_iter;

            if(end < std::string::npos)
                start = end + 1;
        }
        while(end < std::string::npos && elem_iter != elem.end());

        if(elem_iter != elem.end())
            return {
                ErrorCode::invalid_file_formatting,
                std::move(patch_indexes),
                std::move(ctrl_points)
            };

        patch_indexes.push_back(std::move(elem));
        ++patch_count;
    }

    if(patch_count != num_of_patches)
        return {
            ErrorCode::invalid_file_formatting,
            std::move(patch_indexes),
            std::move(ctrl_points)
        };


    size_t num_of_ctrl_points {};

    if(!(input_file >> num_of_ctrl_points))
        return {
            ErrorCode::invalid_file_formatting,
            std::move(patch_indexes),
            std::move(ctrl_points)
        };

    ctrl_points.reserve(num_of_ctrl_points);

    CartPoint3d point {};

    for(unsigned i {}; i < num_of_ctrl_points && (input_file >> point); ++i)
        ctrl_points.push_back(point);

    if(ctrl_points.size() != num_of_ctrl_points)
        return {
            ErrorCode::invalid_file_formatting,
            std::move(patch_indexes),
            std::move(ctrl_points)
        };


    return {
        ErrorCode::success,
        std::move(patch_indexes),
        std::move(ctrl_points)
    };
}

static ErrorCode bezier_writer(const string &out_fn, const string &in_fn, unsigned tesselation_level){

    auto const& [code, patch_indexes, ctrl_points] { read_patch_file(in_fn) };
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

    const Matrix<double, 4, 4> coeffs_matrix {
        array{ -1.0,  3.0, -3.0, 1.0 },
        array{  3.0, -6.0,  3.0, 0.0 },
        array{ -3.0,  3.0,  0.0, 0.0 },
        array{  1.0,  0.0,  0.0, 0.0 }
    };
    const Matrix<double, 4, 4> t_coeffs_matrix { coeffs_matrix.transpose() };

    for(auto const& indexes_array : patch_indexes){

        vector<vector<CartPoint3d>> vertex_matrix {};
        vertex_matrix.reserve(tesselation_level);

        vector<vector<CartPoint3d>> normals_matrix {};
        normals_matrix.reserve(tesselation_level);


        auto iter { indexes_array.begin() };
        const Matrix<CartPoint3d, 4, 4> point_matrix {
            array{
                ctrl_points[*iter++],
                ctrl_points[*iter++],
                ctrl_points[*iter++],
                ctrl_points[*iter++]
            },
            array{
                ctrl_points[*iter++],
                ctrl_points[*iter++],
                ctrl_points[*iter++],
                ctrl_points[*iter++]
            },
            array{
                ctrl_points[*iter++],
                ctrl_points[*iter++],
                ctrl_points[*iter++],
                ctrl_points[*iter++]
            },
            array{
                ctrl_points[*iter++],
                ctrl_points[*iter++],
                ctrl_points[*iter++],
                ctrl_points[*iter]
            }
        };

        auto const inner_partial_matrix {
            coeffs_matrix * point_matrix * t_coeffs_matrix
        };

        for(unsigned u{}; u < tesselation_level; ++u){

            vector<CartPoint3d> vertex_row {};
            vertex_row.reserve(tesselation_level);

            vector<CartPoint3d> normals_row {};
            normals_row.reserve(tesselation_level);


            const double u_time { time_step * static_cast<double>(u) };

            const Matrix<double, 1, 4> u_vector { //Bernstein polynomials definiton
                array{
                    u_time * u_time * u_time,
                    u_time * u_time,
                    u_time,
                    1.0
                }
            };

            const Matrix<double, 1, 4> u_vector_deriv {
                array{
                    3.0 * u_time * u_time,
                    2.0 * u_time,
                    1.0,
                    0.0
                }
            };

            for(unsigned v{}; v < tesselation_level; ++v){

                const double v_time { time_step * static_cast<double>(v) };

                const Matrix<double, 4, 1> v_vector {
                    array{ v_time * v_time * v_time},
                    array{ v_time * v_time },
                    array{ v_time },
                    array{ 1.0 }
                };

                const Matrix<double, 4, 1> v_vector_deriv {
                    array{ 3.0 * v_time * v_time },
                    array{ 2.0 * v_time },
                    array{ 1.0 },
                    array{ 0.0 }
                };

                const Matrix<CartPoint3d, 1, 1> vertex {
                    u_vector * inner_partial_matrix * v_vector
                };

                const Matrix<CartPoint3d, 1, 1> tangent1 {
                    u_vector_deriv * inner_partial_matrix * v_vector
                };

                const Matrix<CartPoint3d, 1, 1> tangent2 {
                    u_vector * inner_partial_matrix * v_vector_deriv
                };

                vertex_row.push_back(vertex.at(0, 0));
                normals_row.push_back(
                    cross_product(
                        tangent2.at(0, 0),
                        tangent1.at(0, 0)
                    ).normalize()
                );
            }

            vertex_matrix.push_back(std::move(vertex_row));
            normals_matrix.push_back(std::move(normals_row));
        }

        for(unsigned i{}; i < static_cast<size_t>(tesselation_level - 1); ++i)
            for(unsigned j{}; j < static_cast<size_t>(tesselation_level - 1); ++j){

                vertexes_file << vertex_matrix[i][j]
                              << vertex_matrix[i][j + 1]
                              << vertex_matrix[i + 1][j];

                vertexes_file << vertex_matrix[i + 1][j + 1]
                              << vertex_matrix[i + 1][j]
                              << vertex_matrix[i][j + 1];


                normals_file << normals_matrix[i][j]
                             << normals_matrix[i][j + 1]
                             << normals_matrix[i + 1][j];

                normals_file << normals_matrix[i + 1][j + 1]
                             << normals_matrix[i + 1][j]
                             << normals_matrix[i][j + 1];
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

    std::ofstream text_file{};
    text_file.open(to_text_extension(filename), std::ios::out | std::ios::trunc);

    if(!vertexes_file.is_open() || !normals_file.is_open() || !text_file.is_open())
        return ErrorCode::io_error;

    const double abs_max_coord { static_cast<double>(units) / 2.0 };
    const double step { static_cast<double>(units) / static_cast<double>(grid_size) };
    const double text_step { 1.0 / static_cast<double>(grid_size) };


    CartPoint3d p1 {}, p2 {}, p3 {}, p4 {};
    double x {}, y {}, z {};


    x = abs_max_coord;

    CartPoint3d normal1 { 0.0,  1.0, 0.0 };
    CartPoint3d normal2 { 0.0, -1.0, 0.0 };

    CartPoint2d t1 {}, t2 {}, t3 {}, t4 {};
    double s {}, t {};  //coordinates in texture space


    s = 1.0;

    for(int i{}; i < grid_size; ++i, x -= step, s -= text_step){

        z = abs_max_coord;
        t = 1.0;

        for(int j{}; j < grid_size; ++j, z -= step, t -= text_step){

            p1.y = p2.y = p3.y = p4.y = abs_max_coord;

            p1.x = x; p1.z = z;
            p2.x = x; p2.z = z - step;
            p3.x = x - step; p3.z = z;
            p4.x = x - step; p4.z = z - step;

            vertexes_file << p1 << p2 << p4;
            vertexes_file << p4 << p3 << p1;

            p1.y = p2.y = p3.y = p4.y = -abs_max_coord;

            vertexes_file << p1 << p3 << p2;
            vertexes_file << p2 << p3 << p4;


            normals_file << normal1 << normal1 << normal1;
            normals_file << normal1 << normal1 << normal1;

            normals_file << normal2 << normal2 << normal2;
            normals_file << normal2 << normal2 << normal2;


            t1.x = s; t1.y = t;
            t2.x = s; t2.y = t - text_step;
            t3.x = s - text_step; t3.y = t;
            t4.x = s - text_step; t4.y = t - text_step;

            text_file << t1 << t2 << t4;
            text_file << t4 << t3 << t1;

            text_file << t1 << t3 << t2;
            text_file << t2 << t3 << t4;
        }
    }

    y = abs_max_coord;

    normal1 = { 1.0,  0.0, 0.0 };
    normal2 = { -1.0, 0.0, 0.0 };

    s = 1.0;

    for(int i{}; i < grid_size; ++i, y -= step, s -= text_step){

        z = abs_max_coord;
        t = 1.0;

        for(int j{}; j < grid_size; ++j, z -= step, t -= text_step){

            p1.x = p2.x = p3.x = p4.x = abs_max_coord;

            p1.y = y; p1.z = z;
            p2.y = y; p2.z = z - step;
            p3.y = y - step; p3.z = z;
            p4.y = y - step; p4.z = z - step;

            vertexes_file << p1 << p4 << p2;
            vertexes_file << p1 << p3 << p4;

            p1.x = p2.x = p3.x = p4.x = -abs_max_coord;

            vertexes_file << p1 << p2 << p3;
            vertexes_file << p2 << p4 << p3;


            normals_file << normal1 << normal1 << normal1;
            normals_file << normal1 << normal1 << normal1;

            normals_file << normal2 << normal2 << normal2;
            normals_file << normal2 << normal2 << normal2;


            t1.x = s; t1.y = t;   // t -> y; s -> z
            t2.x = s - text_step; t2.y = t;
            t3.x = s; t3.y = t - text_step;
            t4.x = s - text_step; t4.y = t - text_step;

            text_file << t1 << t4 << t2;
            text_file << t1 << t3 << t4;

            text_file << t1 << t2 << t3;
            text_file << t2 << t4 << t3;
        }
    }

    y = abs_max_coord;

    normal1 = { 0.0, 0.0, 1.0  };
    normal2 = { 0.0, 0.0, -1.0 };

    s = 1.0;

    for(int i{}; i < grid_size; i++, y -= step, s -= text_step){

        x = abs_max_coord;
        t = 1.0;

        for(int j{}; j < grid_size; j++, x -= step, t -= text_step){

            p1.z = p2.z = p3.z = p4.z = abs_max_coord;

            p1.y = y; p1.x = x;
            p2.y = y; p2.x = x - step;
            p3.y = y - step; p3.x = x;
            p4.y = y - step; p4.x = x - step;

            vertexes_file << p1 << p2 << p4;
            vertexes_file << p1 << p4 << p3;

            p1.z = p2.z = p3.z = p4.z = -abs_max_coord;

            vertexes_file << p1 << p3 << p2;
            vertexes_file << p2 << p3 << p4;


            normals_file << normal1 << normal1 << normal1;
            normals_file << normal1 << normal1 << normal1;

            normals_file << normal2 << normal2 << normal2;
            normals_file << normal2 << normal2 << normal2;


            t1.x = s; t1.y = t; //t -> y; s -> x
            t2.x = s - text_step; t2.y = t;
            t3.x = s; t3.y = t - text_step;
            t4.x = s - text_step; t4.y = t - text_step;

            text_file << t1 << t2 << t4;
            text_file << t1 << t4 << t3;

            text_file << t1 << t3 << t2;
            text_file << t2 << t3 << t4;
        }
    }

    return ErrorCode::success;
}

static ErrorCode plane_writer(const string &filename, int length, int divs){

    std::ofstream vertexes_file{};
    vertexes_file.open(filename, std::ios::out | std::ios::trunc);

    std::ofstream normals_file{};
    normals_file.open(to_norm_extension(filename), std::ios::out | std::ios::trunc);

    std::ofstream text_file{};
    text_file.open(to_text_extension(filename), std::ios::out | std::ios::trunc);

    if(!vertexes_file.is_open() || !normals_file.is_open() || !text_file.is_open())
        return ErrorCode::io_error;

    const double abs_max_coord { static_cast<double>(length) / 2.0 };
    const double step { static_cast<double>(length) / static_cast<double>(divs) };
    const double text_step { 1.0 / static_cast<double>(divs) };

    CartPoint3d p1 {}, p2 {}, p3 {}, p4 {};
    const CartPoint3d normal { 0.0, 1.0, 0.0 };           //constant for all points of the plane
    CartPoint2d t1 {}, t2 {}, t3 {}, t4 {};

    double x { abs_max_coord };
    double s { 1.0 };

    for(int i{}; i < divs; ++i, x -= step, s -= text_step){

        double z { abs_max_coord };
        double t { 1.0 };

        for(int j{}; j < divs; ++j, z -= step, t -= text_step){

            p1.x = x; p1.z = z;
            p2.x = x; p2.z = z - step;
            p3.x = x - step; p3.z = z;
            p4.x = x - step; p4.z = z - step;

            vertexes_file << p1 << p4 << p3;
            vertexes_file << p1 << p2 << p4;


            normals_file << normal << normal << normal;
            normals_file << normal << normal << normal;


            //t1.x = s; t1.y = t;
            //t2.x = s; t2.y = t - text_step;
            //t3.x = s - text_step; t3.y = t;
            //t4.x = s - text_step; t4.y = t - text_step;

            t1.x = p1.x; t1.y = p1.z;
            t2.x = p2.x; t2.y = p2.z;
            t3.x = p3.x; t3.y = p3.z;
            t4.x = p4.x; t4.y = p4.z;

            text_file << t1 << t4 << t3;
            text_file << t1 << t2 << t4;
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
        const int in_radius  { string_to_uint(args[args_index++]) };
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