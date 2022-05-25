#!/bin/bash

DIR=$(dirname $BASH_SOURCE)

GEN=$DIR/../bin/generator
RESOURCES=$DIR/../resources

main(){

    if [[ -f bin/generator ]]
    then
        $GEN bezier $RESOURCES/teapot.patch 10 $RESOURCES/bezier.3d
        return 0
    else
        echo "error: bin/generator not found" 1>&2
        return 1
    fi
}

main