#!/bin/bash

DIR=$(dirname $BASH_SOURCE)

GEN=$DIR/../bin/generator
RESOURCES=$DIR/../resources

main(){

    if [[ -f $GEN ]]
    then
        $GEN cone 1 2 4 3 $RESOURCES/cone.3d
        $GEN sphere 1 10 10 $RESOURCES/sphere.3d
        $GEN box 2 3 $RESOURCES/box.3d
        $GEN plane 10 3 $RESOURCES/plane.3d
        $GEN torus 5 1 20 20 $RESOURCES/torus.3d
        return 0
    else
        echo "error: bin/generator not found" 1>&2
        return 1
    fi
}

main