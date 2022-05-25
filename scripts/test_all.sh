#!/bin/bash

DIR=$(dirname $BASH_SOURCE)

ENG=$DIR/../bin/engine
RESOURCES=$DIR/../resources

OK=0

terminate(){
    echo "bin/engine exited with error code"
    exit 1
}

run(){
    $1
    OK=$?

    if [ $OK -eq 1 ]
    then
        exit 1
    fi
}

main(){

    if [[ -f $ENG ]]
    then

        run $DIR/test1.sh
        run $DIR/test2.sh
        run $DIR/test3.sh
        run $DIR/test4.sh

        for test_file in $RESOURCES/solar*.xml
        do
            echo "Running bin/engine $test_file..."
            $ENG $test_file y 100

            if [[ $? -eq 1 ]]
            then
                terminate
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