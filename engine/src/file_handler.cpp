#include "file_handler.hpp"

using std::vector;
using std::string;
using std::tuple;



template<typename T>
static tuple<ErrorCode, vector<T>> file_reader(const string& fn){

    static_assert(
        std::is_same<T, CartPoint3d>::value ||
        std::is_same<T, CartPoint2d>::value
    );

    vector<T> points {};

    std::ifstream file{};
    file.open(fn, std::ios::in);

    if(!file.is_open())
        return {
            ErrorCode::io_error,
            std::move(points)
        };


    T p1{}, p2{}, p3{};
    while(file >> p1 >> p2 >> p3){
        points.push_back(p1);
        points.push_back(p2);
        points.push_back(p3);
    }

    return {
        ErrorCode::success,
        std::move(points)
    };
}

tuple<ErrorCode, vector<CartPoint3d>, vector<CartPoint3d>, vector<CartPoint2d>>
files_reader(const string &model_fn){

    auto const& [vcode, vertexes] { file_reader<CartPoint3d>(model_fn) };
    auto const& [ncode, normals] { file_reader<CartPoint3d>(to_norm_extension(model_fn)) };
    auto const& [tcode, text_coords] { file_reader<CartPoint2d>(to_text_extension(model_fn)) };

    static const string warning { "\033[35;1mWarning:\033[0m " };

    if(vcode != ErrorCode::success)
        std::cout << warning << "Unable to load vertexes for model '" << model_fn << "'.\n";

    if(ncode != ErrorCode::success)
        std::cout << warning << "Unable to load normals for model '" << model_fn << "'.\n";

    if(tcode != ErrorCode::success)
        std::cout << warning << "Unable to load texture coordinates for model '" << model_fn << "'.\n";

    return tuple{
        vcode,
        std::move(vertexes),
        std::move(normals),
        std::move(text_coords)
    };
}