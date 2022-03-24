#!/bin/bash

gen(){

    if [[ -f bin/generator ]]
    then
        bin/generator box 2 3 resources/box.3d
        bin/generator cone 1 2 4 3 resources/cone.3d
        bin/generator sphere 1 8 8 resources/sphere.3d
        return 0
    else
        echo "error: bin/generator not found" 1>&2
        return 1
    fi
}

gen