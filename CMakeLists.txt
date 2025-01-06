cmake_minimum_required(VERSION 3.10)

project(Orc_Horde
		VERSION 0.0.1
		DESCRIPTION "Fight the orc hordes with your minions and your friends")
		

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(orc_horde src/main.cpp src/vulkan_renderer.cpp)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)

target_link_libraries(orc_horde PRIVATE glfw Vulkan::Vulkan)
