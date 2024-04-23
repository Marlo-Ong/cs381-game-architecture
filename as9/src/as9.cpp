#include "raylib-cpp.hpp"
#include <concepts>
#include <memory>
#include <optional>
#include <iostream>
#include <BufferedInput.hpp>
#include "ECS.hpp"

template<typename T>
concept Transformer = requires(T t, raylib::Transform m) {
	{ t.operator()(m) } -> std::convertible_to<raylib::Transform>;
};

void DrawBoundedModel(raylib::Model& model, Transformer auto transformer);
void DrawModel(raylib::Model& model, Transformer auto transformer); 

struct TransformComponent {
    raylib::Vector3 position;
    raylib::Quaternion rotation;
    raylib::Vector3 scale;
};

struct Rendering {
    raylib::Model* model;
    bool drawBoundingBox = false;
};

struct Kinematics {
    float minSpeed = -200;
    float maxSpeed =  200;
    float acceleration = 15;
    float angularAcceleration = 10;
    raylib::Vector3 velocity = {0,0,0};
    float speed = 0;
    float targetSpeed = 0;
};

struct Physics2D {
    raylib::Degree heading = 0;
    raylib::Degree targetHeading = 0;
};

struct Physics3D {
    raylib::Quaternion rotationQuat = raylib::Quaternion::Identity();
    raylib::Quaternion targetRotationQuat = raylib::Quaternion::Identity();
};

struct InputComponent {
    raylib::BufferedInput inputs;
};

void DrawSystem(cs381::Scene<>& scene) {
    // SYNTACTICAL SUGAR!
    // for (auto x: someArray) {}
    // auto&& begin = someArray.begin; (SceneView<...>::Iterator&&)
    // auto&& end = someArray.end; (SceneView<...>::Sentinel&&)
    // for (; begin != end; begin++) { auto x = *begin; }
    // uses iterators: class that behaves like a pointer, pointer with extra functionality

    for(auto [rendering, transform]: cs381::SceneView<Rendering, TransformComponent>{scene}) {
        auto [axis, angle] = transform.rotation.ToAxisAngle();
        auto transformer = [transform = transform, axis = axis, angle = angle](raylib::Transform t) -> raylib::Transform {
            return t
                .Translate(transform.position)
                .Scale(transform.scale.x,
                        transform.scale.y,
                        transform.scale.z)
                .Rotate(axis, angle);
        };

        if (rendering.model) {
            if (rendering.drawBoundingBox)
                DrawBoundedModel(*rendering.model, transformer);
            else DrawModel(*rendering.model, transformer);
        }
    }
}

void KinematicsSystem(cs381::Scene<>& scene, float dt) {
    for(auto [kinem, t]: cs381::SceneView<Kinematics, TransformComponent>{scene}) {
        float target = Clamp(kinem.targetSpeed, kinem.minSpeed, kinem.maxSpeed);
        if(kinem.speed < target) kinem.speed += kinem.acceleration * dt;
        else if(kinem.speed > target) kinem.speed -= kinem.acceleration * dt;
        kinem.speed = Clamp(kinem.speed, kinem.minSpeed, kinem.maxSpeed);

        t.position += kinem.velocity * dt;
    }
}

void Physics2DSystem(cs381::Scene<>& scene, float dt) {
    static constexpr auto AngleClamp = [](raylib::Degree angle) -> raylib::Degree {
        float decimal = float(angle) - int(angle);
        int whole = int(angle) % 360;
        whole += (whole < 0) * 360;
        return decimal + whole;
    };

    for(auto [phys, kinem, transform]: cs381::SceneView<Physics2D, Kinematics, TransformComponent>{scene}) {
        float target = AngleClamp(phys.targetHeading);
        float difference = abs(target - phys.heading);
        if(target > phys.heading) {
            if(difference < 180) phys.heading += kinem.angularAcceleration * dt;
            else if(difference > 180) phys.heading -= kinem.angularAcceleration * dt;
        } else if(target < phys.heading) {
            if(difference < 180) phys.heading -= kinem.angularAcceleration * dt;
            else if(difference > 180) phys.heading += kinem.angularAcceleration * dt;
        } 
        if(difference < .5) phys.heading = target; // If the heading is really close to correct 
        phys.heading = AngleClamp(phys.heading);
        phys.targetHeading = AngleClamp(phys.targetHeading);
        raylib::Radian angle = raylib::Degree(phys.heading);
        raylib::Radian angle2 = raylib::Degree(phys.targetHeading-phys.heading);

        kinem.velocity = raylib::Vector3{cos(angle) * kinem.speed, 0, -sin(angle) * kinem.speed};
        transform.rotation = raylib::Quaternion::FromAxisAngle(raylib::Vector3::Up(), angle);
    }
}

void Physics3DSystem(cs381::Scene<>& scene, float dt) {
    static constexpr auto AngleClamp = [](raylib::Degree angle) -> raylib::Degree {
        float decimal = float(angle) - int(angle);
        int whole = int(angle) % 360;
        whole += (whole < 0) * 360;
        return decimal + whole;
    };

    for(auto [physics3D, kinematics, transform]: cs381::SceneView<Physics3D, Kinematics, TransformComponent>{scene}) {
        physics3D.rotationQuat = physics3D.rotationQuat.Slerp(physics3D.targetRotationQuat, kinematics.angularAcceleration * dt);
        kinematics.velocity = raylib::Vector3::Left().RotateByQuaternion(physics3D.rotationQuat) * kinematics.speed; // Y-axis is Forward
        transform.rotation = physics3D.rotationQuat;
        //std::cout << transform.position.x << ", ";
        //std::cout << transform.position.y << ", ";
        //std::cout << transform.position.z << std::endl;
    }
}

raylib::Vector3 SelectionSystem(cs381::Scene<>& scene, int& id) {
    raylib::Vector3 selectedPlanePosition;
    int i = 0;

    for(auto [input, rendering, t]: cs381::SceneView<InputComponent, Rendering, TransformComponent>{scene}) {
        if (i == id) {
            rendering.drawBoundingBox = true;
            input.inputs.PollEvents();
            selectedPlanePosition = t.position;
        }
        else rendering.drawBoundingBox = false;
        i++;
    }

    return selectedPlanePosition;
}

void InputSystem(cs381::Scene<>& scene) {

    // Basic kinematic controls
    for(auto [input, kinem]: cs381::SceneView<InputComponent, Kinematics>{scene}) {
        input.inputs["forward"] =
            raylib::Action::button_set( {raylib::Button::key(KEY_W), raylib::Button::key(KEY_UP)})
                .SetPressedCallback([&kinem = kinem]{
                    kinem.targetSpeed += 10;    
            }).move();

        input.inputs["back"] =
            raylib::Action::button_set( {raylib::Button::key(KEY_S), raylib::Button::key(KEY_DOWN)})
                .SetPressedCallback([&kinem = kinem]{
                    kinem.targetSpeed -= 10;
            }).move();

        input.inputs["stop"] =
            raylib::Action::key(KEY_SPACE)
                .SetPressedCallback([&kinem = kinem]{
                    kinem.targetSpeed = 0;
            }).move();
    }

    // Basic 2D physics controls
    for(auto [input, physics]: cs381::SceneView<InputComponent, Physics2D>{scene}) {
        input.inputs["upheading"] =
            raylib::Action::button_set( {raylib::Button::key(KEY_A), raylib::Button::key(KEY_LEFT)})
                .SetPressedCallback([&physics = physics]{
                    physics.targetHeading += 10;
            }).move();

        input.inputs["backheading"] =
            raylib::Action::button_set( {raylib::Button::key(KEY_D), raylib::Button::key(KEY_RIGHT)})
                .SetPressedCallback([&physics = physics]{
                    physics.targetHeading -= 10;
            }).move();
    }

    // Basic 3D physics controls
    for(auto [input, physics3D]: cs381::SceneView<InputComponent, Physics3D>{scene}) {
        input.inputs["upyaw"] =
            raylib::Action::button_set( {raylib::Button::key(KEY_A), raylib::Button::key(KEY_LEFT)})
                .SetPressedCallback([&physics3D = physics3D]{
                    physics3D.targetRotationQuat = raylib::Quaternion::FromAxisAngle(raylib::Vector3::Up(), 0.5) * (physics3D.targetRotationQuat);
            }).move();

        input.inputs["backyaw"] =
            raylib::Action::button_set( {raylib::Button::key(KEY_D), raylib::Button::key(KEY_RIGHT)})
                .SetPressedCallback([&physics3D = physics3D]{
                    physics3D.targetRotationQuat = raylib::Quaternion::FromAxisAngle(raylib::Vector3::Up(), -0.5) * (physics3D.targetRotationQuat);
            }).move();

        input.inputs["uproll"] =
            raylib::Action::key(KEY_R)
                .SetPressedCallback([&physics3D = physics3D]{
                    physics3D.targetRotationQuat = raylib::Quaternion::FromAxisAngle(raylib::Vector3::Left(), 0.5) * (physics3D.targetRotationQuat);
            }).move();

        input.inputs["backroll"] =
            raylib::Action::key(KEY_F)
                .SetPressedCallback([&physics3D = physics3D]{
                    physics3D.targetRotationQuat = raylib::Quaternion::FromAxisAngle(raylib::Vector3::Left(), -0.5) * (physics3D.targetRotationQuat);
            }).move();

        input.inputs["uppitch"] =
            raylib::Action::key(KEY_Q)
                .SetPressedCallback([&physics3D = physics3D]{
                    physics3D.targetRotationQuat = raylib::Quaternion::FromAxisAngle(raylib::Vector3::Forward(), 0.5) * (physics3D.targetRotationQuat);
            }).move();

        input.inputs["backpitch"] =
            raylib::Action::key(KEY_E)
                .SetPressedCallback([&physics3D = physics3D]{
                    physics3D.targetRotationQuat = raylib::Quaternion::FromAxisAngle(raylib::Vector3::Forward(), -0.5) * (physics3D.targetRotationQuat);
            }).move();
    }
}


///////////////////////////////////////////////////////////////////////////
// MAIN AREA //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

int main() {
	// Create window
	const int screenWidth = 800;
	const int screenHeight = 450;
	raylib::Window window(screenWidth, screenHeight, "CS381 - Assignment 9");

    cs381::Scene<> scene;

    // Set up input bindings for all entities
    InputSystem(scene);

	// Create camera
	auto camera = raylib::Camera(
		raylib::Vector3(0, 120, -500), // Position
		raylib::Vector3(0, 0, 300), // Target
		raylib::Vector3::Up(), // Up direction
		30.0f,
		CAMERA_PERSPECTIVE
	);

    // EXTRA CREDIT: Play audio (wind howling, plane noises)
    InitAudioDevice();
    raylib::Sound mus_PlaneBG("../audio/air-raid.mp3");
    raylib::Sound sfx_PlaneFlap("../audio/flap.wav");
    mus_PlaneBG.Play();

    // Select selectable entity by ID
    int selector = 0;

    // Listen for TAB key
    raylib::BufferedInput inputs;
    inputs["next"] = 
        raylib::Action::key(KEY_TAB)
            .SetPressedCallback([&selector = selector]{
                selector++;
                selector %= 10;
    }).move();

	// Main loop
	bool keepRunning = true;
	while(!window.ShouldClose() && keepRunning) {
        
        // Update variables

		// Rendering
		{
			// Clear screen
			window.ClearBackground(BLACK);

			camera.BeginMode();
			{
                DrawSystem(scene);

                // Poll events
                inputs.PollEvents();
                //raylib::Vector3 pos = SelectionSystem(scene, selector);

                // EXTRA CREDIT: Camera pans toward target
                //camera.SetTarget(pos);
			}
			camera.EndMode();

			// Measure our FPS
			DrawFPS(10, 10);
		}
		window.EndDrawing();
	}

	return 0;
}

void DrawModel(raylib::Model& model, Transformer auto transformer) {
    raylib::Transform backupTransform = model.transform;
    model.transform = transformer(backupTransform);
    model.Draw({});
    model.transform = backupTransform;
}