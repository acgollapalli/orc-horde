
# abort if any command fails
$ErrorActionPreference = "Stop"

cd $PSScriptRoot

if (-not (Test-Path -Path ".\build\")) { 
	mkdir ".\build\"
}

if (-not (Test-Path -Path ".\build\shaders\")) { 
	mkdir ".\build\shaders"
}

# TODO(caleb): replace this with a for loop over all shaders
# in shaders directory
glslc.exe .\shaders\triangle.vert -o .\build\shaders\triangle_vert.spv
glslc.exe .\shaders\triangle.frag -o .\build\shaders\triangle_frag.spv
glslc.exe .\shaders\base.vert -o .\build\shaders\base_vert.spv
glslc.exe .\shaders\base.frag -o .\build\shaders\base_frag.spv
