#include "textures.hpp"

using std::set;
using std::string;
using std::wstring;
using std::shared_ptr;



shared_ptr<TexturesHandler> TexturesHandler::singleton { nullptr };

TexturesHandler::TexturesHandler(const set<string>& texture_fns) :
    images() {

    ilInit();

    const size_t num_of_images { texture_fns.size() };
    this->images.resize(num_of_images);
    ilGenImages(num_of_images, this->images.data());



    unsigned image_count {};

    for(auto const& texture_fn : texture_fns){

        ilBindImage(this->images[image_count]);

        ilLoadImage(texture_fn.c_str()); //bruh??

        const int width { ilGetInteger(IL_IMAGE_WIDTH) };
        const int height { ilGetInteger(IL_IMAGE_HEIGHT) };

        if(width <= 0 || height <= 0)
            continue;

        ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

        const unsigned char* const image_data { ilGetData() };

        glGenTextures(1, this->images.data() + image_count);

        glBindTexture(GL_TEXTURE_2D, this->images.at(image_count));

	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	    glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA,
            width, height, 0, GL_RGBA,
            GL_UNSIGNED_BYTE, image_data
        );

        this->image_info.insert( { texture_fn, image_count } );

        ++image_count;
    }

    if(image_count < num_of_images){
        ilDeleteImages(num_of_images - image_count, this->images.data() + image_count);
        this->images.resize(image_count);
    }
}

shared_ptr<TexturesHandler> TexturesHandler::get_instance(const set<string> &textures_fns){

    if(TexturesHandler::singleton == nullptr)
        TexturesHandler::singleton =
            std::make_shared<TexturesHandler>(
                std::move<TexturesHandler>(
                    { textures_fns }
                )
            );

    return TexturesHandler::singleton;
}

bool TexturesHandler::bind(const std::string& texture_fn) const {

    if(texture_fn == "")
        return false;

    const bool has_value { this->image_info.count(texture_fn) > 0};

    if(has_value){
        unsigned index { this->image_info.at(texture_fn) };
        glBindTexture(GL_TEXTURE_2D, this->images.at(index));
    }

    return has_value;
}

void TexturesHandler::clear() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}