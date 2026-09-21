cmake_minimum_required(VERSION 3.16)
project(ForgeUI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(forgeui INTERFACE)
target_include_directories(forgeui INTERFACE include)

option(FORGEUI_BUILD_EXAMPLES "Build host examples" ON)
if(FORGEUI_BUILD_EXAMPLES)
    add_executable(forgeui_host_demo examples/host_demo/main.cpp)
    target_link_libraries(forgeui_host_demo PRIVATE forgeui)
endif()
