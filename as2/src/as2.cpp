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

    // Load sky
    cs381::SkyBox skybox;
    skybox.Load("../textures/skybox.png");

    // Load ground
    auto mesh = raylib::Mesh::Plane(10'000, 10'000, 50, 50, 25);
    raylib::Model ground = ((raylib::Mesh*)&mesh)->LoadModelFrom();
    raylib::Texture grass("../textures/grass.jpg");
    grass.SetFilter(TEXTURE_FILTER_BILINEAR);
    grass.SetWrap(TEXTURE_WRAP_REPEAT);
    ground.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = grass;

    // Load plane
    raylib::Model plane("../meshes/PolyPlane.glb");
    plane.transform = raylib::Transform(plane.transform).Scale(3,3,3);

    // EXTRA CREDIT: Load custom mesh: BREAD PACK
    raylib::Model bread("../meshes/breadpack.glb");
    bread.transform = raylib::Transform(bread.transform).Scale(5,5,5).RotateZ(raylib::Degree(180));

    // Load camera
    raylib::Camera camera(
        raylib::Vector3(0, 120,-400),  // position
        raylib::Vector3(0, 0, 300),  // target
        raylib::Vector3(0, 1, 0),     // up
        45.0f,                        // fov
        CAMERA_PERSPECTIVE            // projection
    );

    // Glob variables
    raylib::Vector3 velocity = raylib::Vector3(0,0,0);
    float velocityScale = 10.0f;
    raylib::Vector3 position = {0,0,0};

    // EXTRA CREDIT: Play audio (wind howling, plane noises)
    InitAudioDevice();
    raylib::Sound mus_PlaneBG("../audio/air-raid.mp3");
    raylib::Sound sfx_PlaneFlap("../audio/flap.wav");
    mus_PlaneBG.Play();

    while (!window.ShouldClose()) {
        window.BeginDrawing();
        {
            camera.BeginMode();
            {
                window.ClearBackground(raylib::Color::Red());
                skybox.Draw();
                ground.Draw({0,0,0});

                DrawBoundedModel(plane, [&position](raylib::Transform t) -> raylib::Transform{
                    return t.Translate(position);
                });
                DrawBoundedModel(bread, [](raylib::Transform t) -> raylib::Transform{
                    return t.Translate({200,20,0});
                });
                DrawBoundedModel(bread, [](raylib::Transform t) -> raylib::Transform{
                    return t.Translate({-200,20,0});
                });

                position += velocity * window.GetFrameTime();

                // Controls
                if (GetKeyPressed()) sfx_PlaneFlap.Play();
                if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) velocity += raylib::Vector3::Forward() * velocityScale;
                if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) velocity += raylib::Vector3::Back() * velocityScale;
                if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) velocity += raylib::Vector3::Left() * velocityScale;
                if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) velocity += raylib::Vector3::Right() * velocityScale;
                if (IsKeyPressed(KEY_Q)) velocity += raylib::Vector3::Up() * velocityScale;
                if (IsKeyPressed(KEY_E)) velocity += raylib::Vector3::Down() * velocityScale;
                if (IsKeyPressed(KEY_SPACE)) velocity = raylib::Vector3(0,0,0);

                // EXTRA CREDIT: Camera pans toward target (plane)
                camera.SetTarget(position);
            }
            camera.EndMode();
            DrawFPS(0,0);
        }
        window.EndDrawing();
    }

    return 0;
}
