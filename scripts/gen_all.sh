#!/bin/bash

DIR=$(dirname $BASH_SOURCE)

GEN=$DIR/../bin/generator

main(){

    if [[ -f $GEN ]]
    then
        $DIR/gen1.sh
        $DIR/gen2.sh
        $DIR/gen3.sh
        $DIR/gen4.sh
        return 0
    else
        echo "error: bin/generator not found" 1>&2
        return 1
    fi
}

main