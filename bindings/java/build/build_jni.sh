#!/bin/bash

cd ../jni
gcc -Wall -shared -fPIC -o libusc.so -I${JAVA_HOME}/include -I../class usc.c -lusc
