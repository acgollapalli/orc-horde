# abort if any command fails
$ErrorActionPreference = "Stop"

cd $PSScriptRoot

if (-not (Test-Path -Path ".\build" )){
	mkdir ".\build"
}

# compile shaders
echo "\n----COMPILING SHADERS---\n"
./compile_shaders.ps1

$env:glfw3_DIR="C:\Program Files (x86)\GLFW\lib\cmake\"

# compile game
echo "\n---COMPILING GAME---\n"
cd build
cmake.exe -DCMAKE_BUILD_TYPE=Debug ..
cmake.exe --build .

# TODO(caleb): make CMake copy the shaders folder to the build target folder

cp -r -Force shaders Debug/

if ($args[0] -eq "run") {
	c:\raddbg\raddbg.exe .\Debug\orc_horde.exe
}

cd ..
