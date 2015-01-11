#!/bin/bash

phpize
LDFLAGS=-lusc ./configure --enable-usc
make

