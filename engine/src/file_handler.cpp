#include "file_handler.hpp"

using std::vector;
using std::string;



static ErrorCode file_reader(const string& fn, vector<CartPoint3d> &points){

    std::ifstream file{};
    file.open(fn, std::ios::in);

    if(!file.is_open())
        return ErrorCode::io_error;


    CartPoint3d p1{}, p2{}, p3{};
    while(file >> p1 >> p2 >> p3){
        points.push_back(p1);
        points.push_back(p2);
        points.push_back(p3);
    }

    return ErrorCode::success;
}



ErrorCode files_reader(const string &model_fn,
                       vector<CartPoint3d> &vertexes,
                       vector<CartPoint3d> &normals){

    const ErrorCode code { file_reader(model_fn, vertexes) };

    if(code == ErrorCode::success)
        return file_reader(to_norm_extension(model_fn), normals);

    return code;
}