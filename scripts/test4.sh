#!/bin/bash

DIR=$(dirname $BASH_SOURCE)

ENG=$DIR/../bin/engine
RESOURCES=$DIR/../resources

main(){

    if [[ -f $ENG ]]
    then

        for test_file in $RESOURCES/test_4*.xml
        do
            echo "Running bin/engine $test_file..."
            $ENG $test_file y 100

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

main
