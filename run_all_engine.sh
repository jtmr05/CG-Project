#!/bin/bash

run(){

    if [[ -f bin/engine ]]
    then

        for test_file in resources/*.xml
        do
            echo "Running bin/engine $test_file..."
            bin/engine $test_file y 100

            if [[ $? -eq 1 ]]
            then
                echo "bin/engine exited with error code"
                return 1
            fi

            echo -e "Done\n"
        done

        return 0
    else
        echo "error: bin/engine not found" 1>&2
        return 1
    fi
}

run
