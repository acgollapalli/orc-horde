#!/bin/sh

# abort if any command fails
set -e

# TODO: write build.bat

cd $(dirname "$SCRIPT")

if [ ! -d "build" ]; then
	mkdir build
fi

# compile shaders
echo "\n----COMPILING SHADERS---\n"
./compile_shaders.sh

# compile game
echo "\n---COMPILING GAME---\n"
cd build
cmake ..
cmake --build .

if [ "$1" = 'run' ]; then
	./orc_horde
fi
