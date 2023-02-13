#!/bin/bash

[ ! -d bin ] && mkdir -p bin | echo 

echo "Loading..."
gcc src/processA.c -lbmp -lncurses -lm -o bin/processA -lrt -pthread
gcc src/SERVER.c -lncurses -lbmp -lm -o bin/SERVER -lrt -pthread
gcc src/CLIENT.c -lncurses -lbmp -lm -o bin/CLIENT -lrt -pthread
gcc src/processB.c -lbmp -lncurses -lm -o bin/processB -lrt -pthread
gcc src/master.c -lbmp -lncurses -lm -o bin/master -lrt -pthread  

echo "The program will start..."
./bin/master
