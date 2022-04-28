#ifndef VBO_HPP
#define VBO_HPP

#include <vector>
#include <set>
#include <map>
#include <string>
#include <fstream>

#include <GL/glew.h>

#include "point.hpp"


class VBO{

    private:
        static VBO* singleton;
        std::vector<unsigned> buffers;

        /**
         * For each model, map a pair containing its index and corresponding size,
         * i.e. the number of points to be drawn
         */
        std::map<std::string, std::pair<unsigned, size_t>> model_index_mappings;

        VBO(const std::set<std::string> &models);


    public:
        static VBO* get_instance(const std::set<std::string> &models);
        bool render(const std::string &model) const;
};

#endif