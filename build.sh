#!/bin/sh

# abort if any command fails
set -e

# TODO: write build.bat

cd $(dirname "$SCRIPT")

if [ ! -d "build" ]; then
	mkdir build
fi

export CC=/usr/bin/gcc-14
export CXX=/usr/bin/g++-14

# compile shaders
echo "\n----COMPILING SHADERS---\n"
./compile_shaders.sh

# compile game
echo "\n---COMPILING GAME---\n"
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .

if [ "$1" = 'run' ]; then
	./orc_horde
fi
