cmake_minimum_required(VERSION 3.18)
project(as5 LANGUAGES CXX)
add_subdirectory(raylib-cpp)
file(GLOB sources "src/*.cpp" "src/*.c")
add_executable(as5 ${sources})
target_link_libraries(as5 PUBLIC raylib raylib_cpp raygui raylib::buffered)
set_property(TARGET as5 PROPERTY CXX_STANDARD 20)

configure_file(textures/skybox.png textures/skybox.png COPYONLY)
configure_file(textures/grass.jpg textures/grass.jpg COPYONLY)

include(includeable.cmake)

make_includeable(raylib-cpp/raylib/examples/models/resources/shaders/glsl330/skybox.vs generated/skybox.vs)
make_includeable(raylib-cpp/raylib/examples/models/resources/shaders/glsl330/skybox.fs generated/skybox.fs)
make_includeable(raylib-cpp/raylib/examples/models/resources/shaders/glsl330/cubemap.vs generated/cubemap.vs)
make_includeable(raylib-cpp/raylib/examples/models/resources/shaders/glsl330/cubemap.fs generated/cubemap.fs)