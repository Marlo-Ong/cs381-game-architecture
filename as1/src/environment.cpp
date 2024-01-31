#include "raylib-cpp.hpp"
#include "rlgl.h"

struct Skybox {
    raylib::Texture texture;
    raylib::Shader shader;
    raylib::Model cube;

    Skybox(): shader(0) {}; // default constructor

    void Init() {
        auto gen = raylib::Mesh::Cube(1,1,1);
        cube = ((raylib::Mesh*)(&gen))->LoadModelFrom();

        // C++ raw string syntax: R"{delimiter}(){delimiter}
        shader = raylib::Shader::LoadFromMemory(
            #include "../generated/skybox.vs"
            ,
            #include "../generated/skybox.fs"
        );
        cube.materials[0].shader = shader;
        shader.SetValue("environmentMap", (int)MATERIAL_MAP_CUBEMAP, SHADER_UNIFORM_INT);
    }

    void Load(std::string filename) {
        shader.SetValue("doGamma", 0, SHADER_UNIFORM_INT);
        shader.SetValue("vflipped", 0, SHADER_UNIFORM_INT);

        raylib::Image img(filename);
        texture.Load(filename, CUBEMAP_LAYOUT_AUTO_DETECT);
        texture.SetFilter(TEXTURE_FILTER_BILINEAR); // blends pixels (as opposed to point filtering)

        cube.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = texture;
    }

    void Draw() {
        rlDisableBackfaceCulling(); // get rid of inside of objects; only render outward-facing faces
        rlDisableDepthMask();
        cube.Draw();
        rlEnableBackfaceCulling();
        rlEnableDepthMask();
    }
};

int main() {
    raylib::Window window(1000,1000, "CS381 - Assignment 0");

    raylib::Model defaultCube("bad.obj");
    raylib::Camera camera(
        raylib::Vector3(0, 120, -500),  // position
        raylib::Vector3(0, 0, 300),     // target
        raylib::Vector3::Up(),          // up
        45,                             // fov
        CAMERA_PERSPECTIVE              // projection
    );

    SetWindowState(FLAG_WINDOW_RESIZABLE);
    raylib::Font text;

    // Skybox skybox;
    // skybox.Load("../textures/skybox.png");

    while (!window.ShouldClose()) {
        window.BeginDrawing();
        {
            camera.BeginMode();
            {
                // skybox.Draw();
                defaultCube.Draw({0,0,0}); // xzy plane; z is vertical, y is into the screen
            }
            camera.EndMode();
        }
        window.EndDrawing();
    }

    return 0;
}
