#include "raylib-cpp.hpp"

// Some object that can be used as a function, and takes in a transform, and returns a transform
template <typename T>
concept Transformer = requires(T t, raylib::Transform m) {
    { t.operator()(m) } -> std::convertible_to<raylib::Transform>;
};

// auto = templated (T t) argument in function def

void DrawBoundedModel(raylib::Model& model, Transformer auto transformer) {
// equivalent to:
// template <Transformer T>
// void DrawBoundedModel(raylib:Model& model, T transformer) {

    raylib::Transform backupTransform = model.transform;
    model.transform = transformer(backupTransform);
    model.Draw({});
    model.GetTransformedBoundingBox().Draw();
    model.transform = backupTransform;
}

int main() {
    raylib::Window window(600,400, "CS381 - Assignment 1");

    raylib::Model ship("../meshes/ddg51.glb");
    ship.transform = raylib::Transform(ship.transform).RotateXYZ(raylib::Degree(90), 0, raylib::Degree(90));

    raylib::Model plane("../meshes/PolyPlane.glb");
    plane.transform = raylib::Transform(plane.transform).Scale(3,3,3);

    // Free custom mesh (breadpack.glb) via SketchFab.com
    raylib::Model bread("../meshes/breadPack.glb");
    bread.transform = raylib::Transform(bread.transform).Scale(3,3,3);

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
            window.ClearBackground(BLACK);
            camera.BeginMode();
            {
                DrawBoundedModel(plane, [](raylib::Transform t) -> raylib::Transform{
                    return t.Translate({0,0,0});
                });
                DrawBoundedModel(plane, [](raylib::Transform t) -> raylib::Transform{
                    return t.Translate({-100,100,0}).Scale(1, -1, 1).RotateY(raylib::Degree(180));
                });

                DrawBoundedModel(ship, [](raylib::Transform t) -> raylib::Transform{
                    return t.Translate({-200,0,0});
                });
                DrawBoundedModel(ship, [](raylib::Transform t) -> raylib::Transform{
                    return t.Translate({200,0,0}).RotateY(raylib::Degree(90));
                });
                DrawBoundedModel(ship, [](raylib::Transform t) -> raylib::Transform{
                    return t.Translate({100,100,0}).Scale(1,2,1).RotateY(raylib::Degree(270));
                });

                // Extra credit: Integrate custom mesh (+BIG BREAD)
                DrawBoundedModel(bread, [](raylib::Transform t) -> raylib::Transform{
                    return t.Translate({-150,150,0}).Scale(3,3,3);
                });

                // Extra credit: Animate your ships (+LARGE ROTATING BREAD)
                raylib::Degree frameRotation = raylib::Degree(window.GetFrameTime());
                bread.transform = raylib::Transform(bread.transform).RotateX(5.0f * frameRotation);
                ship.transform = raylib::Transform(ship.transform).RotateY(2.0f * frameRotation);
                
            }
            camera.EndMode();
        }
        window.EndDrawing();
    }

    return 0;
}
