#!/bin/bash

gen(){

    if [[ -f bin/generator ]]
    then
        bin/generator cone 1 2 4 3 resources/cone.3d
        bin/generator sphere 1 10 10 resources/sphere.3d
        bin/generator box 2 3 resources/box.3d
        bin/generator plane 3 3 resources/plane.3d
        bin/generator torus 5 1 20 20 resources/torus.3d
        return 0
    else
        echo "error: bin/generator not found" 1>&2
        return 1
    fi
}

gen