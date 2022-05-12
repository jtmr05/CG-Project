#include "file_handler.hpp"

using std::vector;
using std::string;

ErrorCode point_reader(const string &model_fn, vector<CartPoint3d> &vertexes
                       /*vector<CartPoint3d> &normals, vector<CartPoint3d> &text_coords*/){

    std::ifstream file{};
    file.open(model_fn, std::ios::in);

    if(!file.is_open())
        return ErrorCode::io_error;



    CartPoint3d p1{}, p2{}, p3{};

    while(file >> p1 >> p2 >> p3){
        vertexes.push_back(p1);
        vertexes.push_back(p2);
        vertexes.push_back(p3);
    }

    //TODO
    return ErrorCode::success;
}