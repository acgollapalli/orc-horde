#!/bin/sh


# abort if any command fails
set -e

# TODO: write compile_shaders.bat

cd $(dirname "$SCRIPT")
echo $(pwd)

if [ ! -d "build" ]; then
	mkdir build
fi

if [ ! -d "build/shaders" ]; then
	mkdir build/shaders
fi

# TODO(caleb): replace this with a for loop over all shaders
# in shaders directory
glslc shaders/triangle.vert -o build/shaders/triangle_vert.spv
glslc shaders/triangle.frag -o build/shaders/triangle_frag.spv
glslc shaders/base.vert -o build/shaders/base_vert.spv
glslc shaders/base.frag -o build/shaders/base_frag.spv
