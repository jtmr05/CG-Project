#ifndef TEXTURES_HPP
#define TEXTURES_HPP

#include <memory>
#include <set>
#include <string>
#include <map>
#include <vector>

#include <IL/il.h>
#include <IL/ilu.h>
#include <GL/gl.h>


class TexturesHandler {

private:
    static std::shared_ptr<TexturesHandler> singleton;

    std::vector<unsigned> images;
    std::map<std::string, unsigned> image_info;


    TexturesHandler(const std::set<std::string> &texture_fns);

public:
    static std::shared_ptr<TexturesHandler> get_instance(const std::set<std::string> &texture_fns);

    bool bind(const std::string& texture_fn) const;
    void clear() const;
};


#endif