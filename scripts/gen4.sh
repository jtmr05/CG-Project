#!/bin/bash

DIR=$(dirname $BASH_SOURCE)

GEN=$DIR/../bin/generator
RESOURCES=$DIR/../resources

main(){

    if [[ -f $GEN ]]
    then
        $GEN box 2 3 $RESOURCES/box_nt.3d
        $GEN plane 2 3 $RESOURCES/plane_nt.3d
        $GEN cone 1 2 10 10 $RESOURCES/cone_nt.3d
        $GEN sphere 1 10 10 $RESOURCES/sphere_nt.3d
        $GEN sphere 1 32 32 $RESOURCES/sphere_nt2.3d
        $GEN bezier $RESOURCES/teapot.patch 10 $RESOURCES/bezier_nt.3d
        $GEN torus 2 1 10 10 $RESOURCES/torus_nt.3d
        return 0
    else
        echo "error: bin/generator not found" 1>&2
        return 1
    fi
}

main