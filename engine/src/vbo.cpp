#include "vbo.hpp"

using std::string;
using std::vector;
using std::set;
using std::shared_ptr;



shared_ptr<VBO> VBO::singleton { nullptr };

VBO::VBO(const set<string> &model_fns) :
    buffers(), model_info(), normals_info(), text_coords_info(){

    glewInit();

    const size_t num_of_buffers { model_fns.size() * 3 }; //space for normals as well

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

    this->buffers.resize(num_of_buffers);
    glGenBuffers(num_of_buffers, this->buffers.data());



    unsigned buffer_count {};

    for(auto const& model_fn : model_fns){

        auto const& [code, points, normals, text_coords] { files_reader(model_fn) };
        if(code == ErrorCode::success){

            glBindBuffer(GL_ARRAY_BUFFER, this->buffers.at(buffer_count));
            glBufferData(
                GL_ARRAY_BUFFER,
                static_cast<long>(points.size() * sizeof(*points.data())),
                points.data(),
                GL_STATIC_DRAW
            );

            this->model_info.insert( { model_fn, { buffer_count, points.size() } } );
            ++buffer_count;

            if(normals.size() > 0){

                glBindBuffer(GL_ARRAY_BUFFER, this->buffers.at(buffer_count));
                glBufferData(
                    GL_ARRAY_BUFFER,
                    static_cast<long>(normals.size() * sizeof(*normals.data())),
                    normals.data(),
                    GL_STATIC_DRAW
                );

                this->normals_info.insert( { model_fn, { buffer_count, normals.size() } } );
                ++buffer_count;
            }

            if(text_coords.size() > 0){

                glBindBuffer(GL_ARRAY_BUFFER, this->buffers.at(buffer_count));
                glBufferData(
                    GL_ARRAY_BUFFER,
                    static_cast<long>(text_coords.size() * sizeof(*text_coords.data())),
                    text_coords.data(),
                    GL_STATIC_DRAW
                );

                this->text_coords_info.insert( { model_fn, { buffer_count, text_coords.size() } } );
                ++buffer_count;
            }
        }
    }

    if(buffer_count < num_of_buffers){
        glDeleteBuffers(num_of_buffers - buffer_count, this->buffers.data() + buffer_count);
        this->buffers.resize(buffer_count);
    }
}

void VBO::init(const std::set<std::string> &model_fns){
    if(VBO::singleton == nullptr)
        VBO::singleton = std::make_shared<VBO>(std::move<VBO>( { model_fns } ));
}

shared_ptr<VBO> VBO::get_instance(){
    return VBO::singleton;
}

bool VBO::render(const string& model_fn) const {

    //check number of mappings for this key
    const bool has_vertexes { this->model_info.count(model_fn) > 0 };
    const bool has_normals { this->normals_info.count(model_fn) > 0 };
    const bool has_texture { this->has_texture(model_fn) };

    if(!has_normals)
        glDisableClientState(GL_NORMAL_ARRAY);

    if(!has_texture)
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    if(has_vertexes){

        auto const& [vindex, vsize] { this->model_info.at(model_fn) };
        glBindBuffer(GL_ARRAY_BUFFER, this->buffers.at(vindex));
        glVertexPointer(3, GL_DOUBLE, 0, 0);

        if(has_normals){
            auto const& [nindex, nsize] { this->normals_info.at(model_fn) };
            glBindBuffer(GL_ARRAY_BUFFER, this->buffers.at(nindex));
            glNormalPointer(GL_DOUBLE, 0, 0);
        }

        if(has_texture){
            auto const& [tindex, tsize] { this->text_coords_info.at(model_fn) };
            glBindBuffer(GL_ARRAY_BUFFER, this->buffers.at(tindex));
            glTexCoordPointer(2, GL_DOUBLE, 0, 0);
        }

        glDrawArrays(GL_TRIANGLES, 0, vsize);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if(!has_texture)
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    if(!has_normals)
        glEnableClientState(GL_NORMAL_ARRAY);

    return has_vertexes;
}

bool VBO::has_texture(const string &model_fn) const {
    return this->text_coords_info.count(model_fn) > 0;
}

void VBO::enable_client_state() const {
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
}

void VBO::disable_client_state() const {
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}