#include "vbo.hpp"

using std::string;
using std::vector;
using std::set;



VBO* VBO::singleton { nullptr };

VBO::VBO(const set<string> &model_fns) :
    buffers( {} ), model_info( {} ){

    glewInit();

    const size_t size { model_fns.size() * 2 }; //space for normals as well

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
        vector<CartPoint3d> normals {};

        if(files_reader(model_fn, points, normals) == ErrorCode::success){

            glBindBuffer(GL_ARRAY_BUFFER, this->buffers.data()[ind]);
            glBufferData(
                GL_ARRAY_BUFFER,
                static_cast<long>(points.size() * sizeof(*points.data())),
                points.data(),
                GL_STATIC_DRAW
            );

            this->model_info.insert( { model_fn, { ind, points.size() } } );
            ++ind;

            glBindBuffer(GL_ARRAY_BUFFER, this->buffers.data()[ind]);
            glBufferData(
                GL_ARRAY_BUFFER,
                static_cast<long>(normals.size() * sizeof(*normals.data())),
                normals.data(),
                GL_STATIC_DRAW
            );

            this->normal_info.insert( { model_fn, { ind, normals.size() } } );
            ++ind;
        }
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

bool VBO::render(const string& model_fn) const {

    //check number of mappings for this key
    const bool has_value {
        this->model_info.count(model_fn) > 0 &&
        this->normal_info.count(model_fn) > 0
    };

    if(has_value){

        const auto& [pindex, psize] { this->model_info.at(model_fn) };
        const auto& [nindex, nsize] { this->normal_info.at(model_fn) };

        glBindBuffer(GL_ARRAY_BUFFER, this->buffers.at(pindex));
        glVertexPointer(3, GL_DOUBLE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, this->buffers.at(nindex));
        glNormalPointer(GL_DOUBLE, 0, 0);

        glDrawArrays(GL_TRIANGLES, 0, psize);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    return has_value;
}

void VBO::enable_client_state() const {
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
}

void VBO::disable_client_state() const {
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}
