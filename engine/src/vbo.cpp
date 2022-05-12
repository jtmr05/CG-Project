#include "vbo.hpp"

using std::string;
using std::vector;
using std::set;
using std::pair;



VBO* VBO::singleton { nullptr };

VBO::VBO(const set<string> &model_fns) :
    buffers( {} ), model_info( {} ){

    glewInit();
    glEnableClientState(GL_VERTEX_ARRAY);

    const size_t size { model_fns.size() };

    /**
     * Reserve doesn't work as the vector merely holds enough memory for 'size' elements
     * but contains no actual elements (i.e. buffers.size() == 0)
     *
     * The call to glGenBuffers will indeed write elements to the underlying array
     * through the pointer obtained from std::vector::data()
     *
     * However, the std::vector container itself doesn't acknowledge any elements
     * since they weren't added through a call to one of its member functions,
     * meaning buffers.size() appears to be 0
     *
     * Hence, buffers.resize() is the solution, as it fills
     * the newly allocated space with default constructed values
     */

    this->buffers.resize(size);
    glGenBuffers(size, this->buffers.data());



    unsigned ind {};

    for(auto const& model_fn : model_fns){

        vector<CartPoint3d> points {};

        if(point_reader(model_fn, points) != ErrorCode::success)
            continue;

        glBindBuffer(GL_ARRAY_BUFFER, this->buffers.data()[ind]);
        glBufferData(
            GL_ARRAY_BUFFER,
            static_cast<long>(points.size() * sizeof(*points.data())),
            points.data(),
            GL_STATIC_DRAW
        );

        const pair<string, pair<unsigned, size_t>> key_value { model_fn, { ind, points.size() } };
        this->model_info.insert(key_value);

        ++ind;
    }

    if(ind < size){
        glDeleteBuffers(size - ind, this->buffers.data() + ind);
        this->buffers.shrink_to_fit();
    }
}

VBO* VBO::get_instance(const set<string> &model_fns){

    if(VBO::singleton == nullptr)
        VBO::singleton = new VBO{ model_fns };

    return VBO::singleton;
}

bool VBO::render(const string& model) const {

    //check number of mappings for this key
    const bool has_value { this->model_info.count(model) > 0 };

    if(has_value){

        const auto& [index, size] = this->model_info.at(model);

        glBindBuffer(GL_ARRAY_BUFFER, this->buffers.at(index));
        glVertexPointer(3, GL_DOUBLE, 0, 0);
        glDrawArrays(GL_TRIANGLES, 0, size);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    return has_value;
}