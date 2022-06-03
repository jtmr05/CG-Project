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


    TexturesHandler(const std::set<std::string> &textures_fns);

public:
    static void init(const std::set<std::string> &textures_fns);
    static std::shared_ptr<TexturesHandler> get_instance();

    bool bind(const std::string& texture_fn) const;
    void clear() const;
};


#endif