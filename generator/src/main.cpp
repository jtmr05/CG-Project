#include "primitive.hpp"

using std::string;

int main(int argc, const char** argv){

    string args[static_cast<size_t>(argc)];

    for(int i = 0; i < argc; i++){
        args[i].append(argv[i]);
    }

    const ErrorCode exit_code = primitive_writer(args, argc);
    handle_error(exit_code);

    return exit_code;
}










/*
#include <cassert>


int main(){
    PolarPoint3d ppp {};
    CartPoint3d ppc {};

*/        /*
        ppp = { 1.0, 45.0, 45.0};
        ppc = polar_to_cart(ppp);
        std::cout << "x: " << ppc.x << " y: " << ppc.y << " z: " << ppc.z << '\n';
        assert(ppc.x > 0.0 && ppc.y > 0.0 && ppc.z > 0.0);

        ppp = { 1.0, 45.0 + 90.0, 45.0};
        ppc = polar_to_cart(ppp);
        std::cout << "x: " << ppc.x << " y: " << ppc.y << " z: " << ppc.z << '\n';
        assert(ppc.x > 0.0 && ppc.y > 0.0 && ppc.z < 0.0);

        ppp = { 1.0, 45.0 + 90.0 + 90.0, 45.0,};
        ppc = polar_to_cart(ppp);
        std::cout << "x: " << ppc.x << " y: " << ppc.y << " z: " << ppc.z << '\n';
        assert(ppc.x < 0.0 && ppc.y > 0.0 && ppc.z < 0.0);

        ppp = { 1.0, 45.0 + 90.0 + 90.0 + 90.0, 45.0};
        ppc = polar_to_cart(ppp);
        std::cout << "x: " << ppc.x << " y: " << ppc.y << " z: " << ppc.z << '\n';
        assert(ppc.x < 0.0 && ppc.y > 0.0 && ppc.z > 0.0);
        */ /*

        ppp = { 1.0, 45.0, 45.0 + 90.0};
        ppc = polar_to_cart(ppp);
        std::cout << "x: " << ppc.x << " y: " << ppc.y << " z: " << ppc.z << '\n';
        assert(ppc.x > 0.0 && ppc.y < 0.0 && ppc.z > 0.0);

        ppp = { 1.0, 45.0 + 90.0, 45.0 + 90.0};
        ppc = polar_to_cart(ppp);
        std::cout << "x: " << ppc.x << " y: " << ppc.y << " z: " << ppc.z << '\n';
        assert(ppc.x > 0.0 && ppc.y < 0.0 && ppc.z < 0.0);

        ppp = { 1.0, 45.0 + 90.0 + 90.0, 45.0 + 90.0};
        ppc = polar_to_cart(ppp);
        std::cout << "x: " << ppc.x << " y: " << ppc.y << " z: " << ppc.z << '\n';
        assert(ppc.x < 0.0 && ppc.y < 0.0 && ppc.z < 0.0);

        ppp = { 1.0, 45.0 + 90.0 + 90.0 + 90.0, 45.0 + 90.0};
        ppc = polar_to_cart(ppp);
        std::cout << "x: " << ppc.x << " y: " << ppc.y << " z: " << ppc.z << '\n';
        assert(ppc.x < 0.0 && ppc.y < 0.0 && ppc.z > 0.0);
}*/