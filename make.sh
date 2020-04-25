#!/bin/bash

mkdir -p build
gcc -o build/pg_test1 -g -I$PGHOME/include -L$PGHOME/lib pg_test1.c -lpq
gcc -o build/pg_test2 -g -I$PGHOME/include -L$PGHOME/lib pg_test2.c -lpq

