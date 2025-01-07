#!/bin/sh

# TODO: write compile_shaders.bat

cd $(dirname "$SCRIPT")
echo $(pwd)

if [ ! -d "build" ]; then
	mkdir build
fi

if [ ! -d "build/shaders" ]; then
	mkdir build/shaders
fi

glslc shaders/triangle.vert -o build/shaders/triangle_vert.spv
glslc shaders/triangle.frag -o build/shaders/triangle_frag.spv
