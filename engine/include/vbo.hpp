#ifndef VBO_HPP
#define VBO_HPP

#include <vector>
#include <set>
#include <map>
#include <string>
#include <fstream>

#include <GL/glew.h>

#include "point.hpp"
#include "file_handler.hpp"



class VBO{

private:
    static VBO* singleton;
    std::vector<unsigned> buffers;

    /**
     * For each model, map a pair containing its index and corresponding size,
     * i.e. the number of points to be drawn
     */
    std::map<std::string, std::pair<unsigned, size_t>> model_info;
    std::map<std::string, std::pair<unsigned, size_t>> normal_info;


    VBO(const std::set<std::string> &model_fns);

public:
    static VBO* get_instance(const std::set<std::string> &model_fns);
    bool render(const std::string &model_fn) const;

    void enable_client_state() const;
    void disable_client_state() const;
};

#endif