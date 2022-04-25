#!/bin/bash

gen(){

    if [[ -f bin/generator ]]
    then
        bin/generator bezier resources/teapot.patch 10 resources/bezier.3d
        return 0
    else
        echo "error: bin/generator not found" 1>&2
        return 1
    fi
}

gen