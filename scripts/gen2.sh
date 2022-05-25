#!/bin/bash

DIR=$(dirname $BASH_SOURCE)

GEN=$DIR/../bin/generator
RESOURCES=$DIR/../resources

main(){

    if [[ -f $GEN ]]
    then
        $GEN box 2 3 $RESOURCES/box.3d
        $GEN cone 1 2 4 3 $RESOURCES/cone.3d
        $GEN sphere 1 8 8 $RESOURCES/sphere.3d
        $GEN torus 4 2 20 20 $RESOURCES/ring.3d
        return 0
    else
        echo "error: bin/generator not found" 1>&2
        return 1
    fi
}

main