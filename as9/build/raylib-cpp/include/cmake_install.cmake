# Install script for directory: /Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/Library/Developer/CommandLineTools/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/AudioDevice.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/AudioStream.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/BoundingBox.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/Camera2D.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/Camera3D.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/Color.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/Degree.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/Font.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/Functions.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/Gamepad.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/Image.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/Material.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/Matrix.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/Mesh.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/Model.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/ModelAnimation.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/Mouse.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/Music.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/Ray.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/RayCollision.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/RaylibException.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/raylib-cpp-utils.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/raylib-cpp.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/raylib.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/raymath.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/Rectangle.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/RenderTexture.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/Shader.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/Sound.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/Text.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/Texture.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/TextureUnmanaged.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/Touch.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/Vector2.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/Vector3.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/Vector4.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/VrStereoConfig.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/Wave.hpp"
    "/Users/moon/Docs/Code/cs381-game-architecture/as9/raylib-cpp/include/Window.hpp"
    )
endif()

