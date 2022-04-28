#include "vbo.hpp"

using std::string;
using std::vector;
using std::set;
using std::pair;

VBO* VBO::singleton { nullptr };

VBO::VBO(const set<string> &models) : buffers{} {

    glewInit();
    glEnableClientState(GL_VERTEX_ARRAY);

    const size_t size { models.size() };

    this->buffers.resize(size); //shouldn't reserve work?
    glGenBuffers(size, this->buffers.data());



    unsigned ind {};

    for(auto const& m : models){

        std::ifstream file{};
        file.open(m, std::ios::in);

        if(file.is_open()){

            vector<CartPoint3d> points {};
            CartPoint3d p1{}, p2{}, p3{};

            while(file >> p1 >> p2 >> p3){
                points.push_back(p1);
                points.push_back(p2);
                points.push_back(p3);
            }

            glBindBuffer(GL_ARRAY_BUFFER, this->buffers.data()[ind]);
            glBufferData(
                GL_ARRAY_BUFFER,
                static_cast<long>(points.size() * sizeof(*points.data())),
                points.data(),
                GL_STATIC_DRAW
            );

            const pair<string, pair<unsigned, size_t>> key_value { m, { ind, points.size() } };
            this->model_index_mappings.insert(key_value);

            file.close();
            ++ind;
        }
    }

    if(ind < size){
        glDeleteBuffers(size - ind, this->buffers.data() + ind);
        this->buffers.shrink_to_fit();
    }
}

VBO* VBO::get_instance(const set<string> &models){

    if(VBO::singleton == nullptr)
        VBO::singleton = new VBO{ models };

    return VBO::singleton;
}

bool VBO::render(const string& model) const{

    //check number of mappings for this key
    const bool has_value { this->model_index_mappings.count(model) > 0 };

    if(has_value){

        auto&& [index, size] = this->model_index_mappings.at(model);

        glBindBuffer(GL_ARRAY_BUFFER, this->buffers.at(index));
        glVertexPointer(3, GL_DOUBLE, 0, 0);
        glDrawArrays(GL_TRIANGLES, 0, size);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    return has_value;
}