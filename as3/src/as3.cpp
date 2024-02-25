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

void DrawModel(raylib::Model& model, Transformer auto transformer) {
    raylib::Transform backupTransform = model.transform;
    model.transform = transformer(backupTransform);
    model.Draw({});
    model.transform = backupTransform;
}

struct Plane {
    float speed = 0, targetSpeed = 0;
    float ySpeed = 0, targetYSpeed = 0;
    raylib::Degree heading = 0, targetHeading = 0;
    float acceleration = 30; // 60/30 = 2 seconds to turn
    float speedUnits = 20.0f;
    raylib::Vector3 position = {0,0,0};

    Plane(){};

    Plane(Vector3 newPosition) {
        position = newPosition;
    };

    Plane(Vector3 newPosition, float newTargetSpeed, float newTargetYSpeed) {
        position = newPosition;
        targetSpeed = newTargetSpeed;
        targetYSpeed = newTargetYSpeed;
    };

    Plane(const Plane& rhs) {
        float speed = rhs.speed;
        float targetSpeed = rhs.targetSpeed;
        float ySpeed = rhs.ySpeed;
        float targetYSpeed = rhs.targetYSpeed;
        raylib::Degree heading = rhs.heading;
        raylib::Degree targetHeading = rhs.targetHeading;
        float acceleration = rhs.acceleration;
        float speedUnits = rhs.speedUnits;
        raylib::Vector3 position = rhs.position;
    }

    void Move(float deltaTime) {
        raylib::Vector3 velocity = {speed * cos(heading.RadianValue()),     // x: speed * direction
                                    ySpeed,                                 // y: up/down
                                    -speed * sin(heading.RadianValue())};   // z: left hand orientation; must negate

        // Clamp heading
        float difference = abs(targetHeading - heading);
        if (targetHeading > heading) {
            if (difference < 180) heading += acceleration * deltaTime;
            if (difference > 180) heading -= acceleration * deltaTime;
        }
        if (targetHeading < heading) {
            if (difference > 180) heading += acceleration * deltaTime;
            if (difference < 180) heading -= acceleration * deltaTime;
        }

        // Clamp speed
        if (targetSpeed > speed) {
            speed += acceleration * deltaTime;
        }
        if (targetSpeed < speed) {
            speed -= acceleration * deltaTime;
        }
        if (targetYSpeed > ySpeed) {
            ySpeed += acceleration * deltaTime;
        }
        if (targetYSpeed < ySpeed) {
            ySpeed -= acceleration * deltaTime;
        }

        static constexpr auto AngleClamp = [](raylib::Degree angle) -> raylib::Degree { // constexpr: this variable only accessible at compile-time (local function)
            int intpart = angle;
            float floatpart = float(angle) - intpart; // get integer part of float degree to be able to modulo
            intpart %= 360;                           // can return negative degree if less than -360
            intpart += (intpart < 0) * 360;           // normalize -360:+360 to 0:360 -> if degree is negative, add 360
        };

        position += velocity * deltaTime;
    }

    void ResetVelocity() {
        targetSpeed = 0;
        targetYSpeed = 0;
        speed = 0;
        ySpeed = 0;
    }
};

int main() {
    raylib::Window window(600,400, "CS381 - Assignment 3");

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

    // Load planes
    raylib::Model planeModel;
    planeModel.Load("../meshes/PolyPlane.glb");
    planeModel.transform = raylib::Transform(planeModel.transform).Scale(3,3,3);

    int numPlanes = 3;
    Plane plane1 = Plane({0,0,0});
    Plane plane2 = Plane({100,0,100});
    Plane plane3 = Plane({-100,0,-100});

    Plane* selectedPlane = &plane1;
    int planeSelector = 1;

    // EXTRA CREDIT: Load custom mesh: BREAD PACK
    raylib::Model bread("../meshes/breadpack.glb");
    bread.transform = raylib::Transform(bread.transform).Scale(5,5,5).RotateZ(raylib::Degree(180));
    Plane bread1 = Plane({100,0,-100}, 5, 2);
    bread1.heading = 60;

    // Load camera
    raylib::Camera camera(
        raylib::Vector3(0, 120,-400),  // position
        raylib::Vector3(0, 0, 300),  // target
        raylib::Vector3(0, 1, 0),     // up
        45.0f,                        // fov
        CAMERA_PERSPECTIVE            // projection
    );

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
                bread.Draw(bread1.position);

                plane1.Move(window.GetFrameTime());
                plane2.Move(window.GetFrameTime());
                plane3.Move(window.GetFrameTime());
                bread1.Move(window.GetFrameTime());

                switch (planeSelector) {
                    case 1:
                        DrawBoundedModel(planeModel, [&](raylib::Transform t) -> raylib::Transform{
                            return t.Translate(plane1.position).RotateY(plane1.heading);
                        });
                        planeModel.Draw(plane2.position);
                        planeModel.Draw(plane3.position);
                        break;
                    case 2:
                        DrawBoundedModel(planeModel, [&](raylib::Transform t) -> raylib::Transform{
                            return t.Translate(plane2.position).RotateY(plane2.heading);
                        });
                        planeModel.Draw(plane1.position);
                        planeModel.Draw(plane3.position);
                        break;
                    case 0:
                        DrawBoundedModel(planeModel, [&](raylib::Transform t) -> raylib::Transform{
                            return t.Translate(plane3.position).RotateY(plane3.heading);
                        });
                        planeModel.Draw(plane1.position);
                        planeModel.Draw(plane2.position);
                        break;
                }

                // Controls
                if (GetKeyPressed()) sfx_PlaneFlap.Play();
                if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) (*selectedPlane).targetSpeed += (*selectedPlane).speedUnits;
                if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) (*selectedPlane).targetSpeed -= (*selectedPlane).speedUnits;
                if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) (*selectedPlane).targetHeading += (*selectedPlane).speedUnits;
                if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) (*selectedPlane).targetHeading -= (*selectedPlane).speedUnits;

                if (IsKeyPressed(KEY_SPACE)) {
                    (*selectedPlane).ResetVelocity();
                }

                // Select plane
                if (IsKeyPressed(KEY_TAB)) {
                    planeSelector += 1;
                    planeSelector %= numPlanes;

                    switch (planeSelector) {
                        case 1:
                            selectedPlane = &plane1;
                            break;
                        case 2:
                            selectedPlane = &plane2;
                            break;
                        case 0:
                            selectedPlane = &plane3;
                            break;
                    }
                }
                
                // EXTRA CREDIT: Allow plane to fly (y-axis movement)
                if (IsKeyPressed(KEY_Q)) (*selectedPlane).targetYSpeed += (*selectedPlane).speedUnits;
                if (IsKeyPressed(KEY_E)) (*selectedPlane).targetYSpeed -= (*selectedPlane).speedUnits;

                // EXTRA CREDIT: Camera pans toward target (plane)
                camera.SetTarget((*selectedPlane).position);
            }
            camera.EndMode();
            DrawFPS(0,0);
        }
        window.EndDrawing();
    }

    return 0;
}
