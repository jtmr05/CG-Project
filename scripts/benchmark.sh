#!/bin/bash

# Used to benchmark the usage (or not) of
# VBOS for different tesselation levels
# supplied to the bezier primitive

DIR=$(dirname $BASH_SOURCE)

GEN=$DIR/../bin/generator
ENG=$DIR/../bin/engine
RESOURCES=$DIR/../resources

main(){

    if [[ -f $GEN && -f $ENG ]]
    then
        for (( i=8; i < 128; i *= 2 ));
        do
            echo "tesselation level: $i"
            $GEN bezier $RESOURCES/teapot.patch $i $RESOURCES/bezier.3d

            $ENG $RESOURCES/test_3_1.xml n
            if [[ $? -eq 1 ]]
            then
                echo "engine exited with error code"
                return 1
            fi

            $ENG $RESOURCES/test_3_1.xml y
            if [[ $? -eq 1 ]]
            then
                echo "engine exited with error code"
                return 1
            fi
        done
        return 0
    else
        echo "error: bin/generator or bin/engine not found" 1>&2
        return 1
    fi
}

main