cmake_minimum_required(VERSION 3.18)
project(as4 LANGUAGES CXX)
add_subdirectory(raylib-cpp)
file(GLOB sources "src/*.cpp" "src/*.c")
add_executable(as4 ${sources})
target_link_libraries(as4 PUBLIC raylib raylib_cpp)
set_property(TARGET as4 PROPERTY CXX_STANDARD 20)

include(includeable.cmake)