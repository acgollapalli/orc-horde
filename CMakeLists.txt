cmake_minimum_required(VERSION 3.10)

project(Orc_Horde
		VERSION 0.0.1
		DESCRIPTION "Fight the orc hordes with your minions and your friends")
		

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD ON)
set(CXX_FLAGS "-fpermissive")
set(CMAKE_CXX_FLAGS "${CXX_FLAGS}")

add_executable(orc_horde src/main.cpp
						 src/vulkan_renderer.cpp
						 src/vulkan_asset_store.cpp
						 src/vulkan_asset.cpp
						 src/vulkan_mesh.cpp
						 src/vulkan_texture.cpp
						 src/rigid_body.cpp)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)

target_link_libraries(orc_horde PRIVATE glfw Vulkan::Vulkan)
