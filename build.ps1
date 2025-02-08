# abort if any command fails
$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest
$PSNativeCommandUseErrorActionPreference = $true # might be true by default

cd $PSScriptRoot

if (-not (Test-Path -Path ".\build" )){
	mkdir ".\build"
}

# compile shaders
echo "\n----COMPILING SHADERS---\n"
./compile_shaders.ps1

$env:glfw3_DIR="C:\Program Files (x86)\GLFW\lib\cmake\"
$env:sdl3_DIR="C:\SDL3-3.2.0\x86_64-w64-mingw32\lib\cmake\SDL3"

# compile game
echo "\n---COMPILING GAME---\n"
cd build
cmake.exe -DCMAKE_BUILD_TYPE=Debug ..
cmake.exe --build .

# TODO(caleb): make CMake copy the shaders folder to the build target folder

cp -r -Force shaders Debug/
cp -r -Force ../models Debug/
cp -r -Force ../textures Debug/

if ($args[0] -eq "run") {
   cd Debug
   .\orc_horde.exe
   cd ..
}

if ($args[0] -eq "debug") {
	c:\raddbg\raddbg.exe .\Debug\orc_horde.exe
}

cd ..
