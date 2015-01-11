#!/bin/bash

export JAVA_HOME=$(./get_java_home.sh)

./build_class.sh
./build_jni.sh
./run.sh

