#!/bin/bash

DIR=$(dirname $BASH_SOURCE)

GEN=$DIR/../bin/generator
RESOURCES=$DIR/../resources

main(){

    if [[ -f $GEN ]]
    then
        $GEN sphere 1 8 8 $RESOURCES/sphere.3d
        $GEN torus 4 2 20 20 $RESOURCES/ring.3d
        $GEN bezier $RESOURCES/teapot.patch 10 $RESOURCES/bezier.3d
        $GEN sphere 1 32 32 $RESOURCES/sphere_nt2.3d
        $GEN bezier $RESOURCES/teapot.patch 10 $RESOURCES/bezier_nt.3d
        $GEN plane 2 3 $RESOURCES/plane_nt.3d
        return 0
    else
        echo "error: bin/generator not found" 1>&2
        return 1
    fi
}

main