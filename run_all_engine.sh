#!/bin/bash

run(){

    if [[ -f bin/engine ]]
    then
        for test_file in resources/*.xml ; do
            echo "Running $test_file..."
            bin/engine $test_file 100
            echo "Done"
            echo ""
        done
        return 0
    else
        echo "error: bin/engine not found" 1>&2
        return 1
    fi
}

run