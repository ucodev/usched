#!/bin/sh

phpize
LDFLAGS=-lusc ./configure --enable-usc
make

