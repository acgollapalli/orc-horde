#!/bin/sh

# TODO: write build.bat

cd $(dirname "$SCRIPT")

if [ ! -d "build" ]; then
	mkdir build
fi

cd build
cmake ..
../compile_shaders.sh
cmake --build
