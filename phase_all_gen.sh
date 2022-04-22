#!/bin/bash

gen(){

    if [[ -f bin/generator ]]
    then
        ./phase1_gen.sh
        ./phase2_gen.sh
        return 0
    else
        echo "error: bin/generator not found" 1>&2
        return 1
    fi
}

gen