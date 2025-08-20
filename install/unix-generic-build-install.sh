#!/usr/bin/bash
gcc -I../include -c -fpic ../src/carg_impl.c
gcc -shared -o libcarg.so carg_impl.o
rm carg_impl.o
sudo cp libcarg.so /usr/lib/
sudo mkdir -p /usr/include/carg
sudo cp ../include/carg.h /usr/include/carg