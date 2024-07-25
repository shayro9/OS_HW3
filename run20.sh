#!/bin/bash

# Path to the executable file
executable="./client localhost 8003 home.html "

# Loop to execute the file 20 times
for i in {1..200}
do
    echo "Execution $i"
    $executable
done

