#!/bin/bash


# Used to benchmark the usage (or not) of
# VBOS for different tesselation levels
# supplied to the bezier primitive



run(){

    if [[ -f bin/generator && -f bin/engine ]]
    then
        for (( i=8; i < 128; i *= 2 ));
        do
            echo "tesselation level: $i"
            bin/generator bezier resources/teapot.patch $i resources/bezier.3d

            bin/engine resources/test_3_1.xml n
            if [[ $? -eq 1 ]]
            then
                echo "bin/engine exited with error code"
                return 1
            fi

            bin/engine resources/test_3_1.xml y
            if [[ $? -eq 1 ]]
            then
                echo "bin/engine exited with error code"
                return 1
            fi
        done
        return 0
    else
        echo "error: bin/generator or bin/engine not found" 1>&2
        return 1
    fi
}

run