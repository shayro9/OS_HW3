#!/bin/bash

# Path to the executable file

# Loop to execute the file 20 times
for i in {1..10}
do
    echo "Execution $i"
    ./client localhost 8003 home$i.html &
    sleep 0.1
done

