#!/bin/bash

gen(){

    if [[ -f bin/generator ]]
    then
        bin/generator box 2 3 resources/box_nt.3d
        bin/generator plane 2 3 resources/plane_nt.3d
        bin/generator cone 1 2 10 10 resources/cone_nt.3d
        bin/generator sphere 1 10 10 resources/sphere_nt.3d
        bin/generator sphere 1 32 32 resources/sphere_nt2.3d
        bin/generator bezier resources/teapot.patch 10 resources/bezier_nt.3d
        bin/generator torus 2 1 10 10 resources/torus_nt.3d
        return 0
    else
        echo "error: bin/generator not found" 1>&2
        return 1
    fi
}

gen
