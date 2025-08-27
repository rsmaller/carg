#!/usr/bin/bash
gcc -I../include -c -fpic ../src/carg_impl.c
gcc -shared -o libcarg.so carg_impl.o
rm carg_impl.o
sudo cp libcarg.so /usr/local/lib/
sudo mkdir -p /usr/local/include/carg
sudo cp ../include/carg.h /usr/local/include/carg
sudo ldconfig