#include "raylib-cpp.hpp"
#include "../skybox/skybox.cpp"

template <typename T>
concept Transformer = requires(T t, raylib::Transform m) {
    { t.operator()(m) } -> std::convertible_to<raylib::Transform>;
};

void DrawBoundedModel(raylib::Model& model, Transformer auto transformer) {
    raylib::Transform backupTransform = model.transform;
    model.transform = transformer(backupTransform);
    model.Draw({});
    model.GetTransformedBoundingBox().Draw();
    model.transform = backupTransform;
}

int main() {
    raylib::Window window(600,400, "CS381 - Assignment 2");

    cs381::SkyBox skybox;
    skybox.Load("../textures/skybox.png");

    raylib::Model plane("../meshes/PolyPlane.glb");
    plane.transform = raylib::Transform(plane.transform).Scale(3,3,3);

    raylib::Camera camera(
        raylib::Vector3(0, 120,500),  // position
        raylib::Vector3(0, 0, -300),  // target
        raylib::Vector3(0, 1, 0),     // up
        45.0f,                        // fov
        CAMERA_PERSPECTIVE            // projection
    );

    while (!window.ShouldClose()) {
        window.BeginDrawing();
        {
            camera.BeginMode();
            {
                skybox.Draw();
                DrawBoundedModel(plane, [](raylib::Transform t) -> raylib::Transform{
                    return t.Translate({0,0,0});
                });
            }
            camera.EndMode();
        }
        window.EndDrawing();
    }

    return 0;
}
