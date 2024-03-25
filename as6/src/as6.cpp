#include "raylib-cpp.hpp"
#include "skybox.hpp"
#include <concepts>
#include <memory>
#include <optional>
#include <iostream>
#include <BufferedInput.hpp>

struct Component {
    struct Entity* object;

    Component(struct Entity& e) : object(&e) {}

    virtual void setup() {};
    virtual void cleanup() {};
    virtual void tick(float dt) {};
};

struct TransformComponent : public Component {
    using Component::Component; // using the same constructor as Component
    raylib::Vector3 position = {0,0,0};
    raylib::Vector3 rotation = {0,0,0};
    raylib::Vector3 scale = {1, 1, 1};
};

struct Entity {
    std::vector<std::unique_ptr<Component>> components;
    // unique ptr tied to given component, deletes if component is deleted

    Entity() { AddComponent<TransformComponent>(); }
    Entity(const Entity&) = delete; // Compiler error if you try to copy an Entity, use the move constructor instead
    Entity(Entity&& other) : components(std::move(other.components)) {
        for (auto& c : components)
            c->object = this;
    }

    template<std::derived_from<Component> T, typename... Ts> // derived_from<Component> -> only takes types that are children of Component class
    size_t AddComponent(Ts... args) {
        auto c = std::make_unique<T>(*this, std::forward<Ts>(args)...);
        components.push_back(std::move(c));
        return components.size() - 1;
    }

    template<std::derived_from<Component> T>
    std::optional<std::reference_wrapper<T>> GetComponent() { // reference_wrapper allows & references to be placed within vectors, lists
                                                              // std::optional same as [if (x), y = *x ]

        // Optimization: since all Entities have a Transform, skip the loop below
        if constexpr(std::is_same_v<T, TransformComponent>) { 
            T* cast = dynamic_cast<T*>(components[0].get()); // components[0] = transform comp.
            if(cast) return *cast;
        }

        // Loop through components and return the one requested
        for(auto& c : components) {
            T* cast = dynamic_cast<T*>(c.get());
            if(cast) return *cast;
        }

        // Else return null
        return std::nullopt; // null value when using "optional"
    }

    void tick(float dt) {
        for(auto& c : components)
            c->tick(dt);
    }
};

struct RenderingComponent : public Component {
    using Component::Component;
    RenderingComponent(Entity& e, raylib::Model&& model) : Component(e), model(std::move(model)) {};

    raylib::Model model; // assuming set on construction
    bool isBounded = false;
    
    void tick(float dt) override {
        auto ref = object->GetComponent<TransformComponent>(); // try getting optional ref
        if (!ref) return;                                      // if the component exists 
        auto& transform = ref->get();                          // initialize reference

        raylib::Transform backupTransform = model.transform;

        model.transform = raylib::Transform(model.transform)
            .Translate(transform.position)
            .RotateXYZ(transform.rotation * DEG2RAD)
            .Scale(transform.scale.x, transform.scale.y, transform.scale.z);
        model.Draw({});

        if (isBounded) model.GetTransformedBoundingBox().Draw();
        model.transform = backupTransform;
    }
};

struct PhysicsComponent2D : public Component {
    using Component::Component;

    PhysicsComponent2D(Entity& obj, float a, float b, float c, float d, raylib::Vector3 rot = {0,1,0}) : Component(obj) {
        acceleration = a;
        turningRate = b;
        maxSpeed = c;
        minSpeed = d;
        rotationAxis = rot;

        targetSpeed = 0;
        speed = 0;
        targetHeading = raylib::Degree(90);
        heading = raylib::Degree(90);
    };

    float acceleration;
    float turningRate;

    float maxSpeed;
    float minSpeed;
    float targetSpeed;
    float speed;
    raylib::Vector3 velocity;
    raylib::Vector3 rotationAxis;

    raylib::Degree targetHeading;
    raylib::Degree heading;

    void tick(float dt) override {
        auto ref = object->GetComponent<TransformComponent>();
        if (!ref) return;
        auto& transform = ref->get();  

        velocity = CalculateVelocity(dt);
        transform.position = transform.position + velocity * dt;
        transform.rotation = raylib::Vector3(heading, heading, heading).Multiply(rotationAxis).Add(
            transform.rotation.Multiply(raylib::Vector3::One().Subtract(rotationAxis)));
        std::cout <<"(" << transform.rotation.x << ", " << transform.rotation.y << ", " << transform.rotation.z << ") - " <<"(" << transform.position.x << ", " << transform.position.y << ", " << transform.position.z << ")" << std::endl;
    }

    raylib::Vector3 CalculateVelocity(float dt){
        static constexpr auto AngleClamp = [](raylib::Degree angle) -> raylib::Degree {
		    float decimal = float(angle) - int(angle);
		    int whole = int(angle) % 360;
		    whole += (whole < 0) * 360;
		    return decimal + whole;
	    };

        float tempTarget = AngleClamp(targetHeading);
        float tempHead = AngleClamp(heading);

        float difference = abs(tempTarget - tempHead);
        if (tempTarget > tempHead) {
            if (difference < 180) heading += turningRate * dt;
            else if (difference > 180) heading -= turningRate * dt;
        }
        else if (tempTarget < tempHead) {
            if (difference > 180) heading += turningRate * dt;
            else if (difference < 180) heading -= turningRate * dt;
        }
        if(difference < 2) heading = targetHeading;

        if (targetSpeed > speed) speed += acceleration * dt;
        else if (targetSpeed < speed) speed -= acceleration * dt;

        return {speed * sin(heading.RadianValue()), 0, -speed * cos(heading.RadianValue())};
    }

    void IncreaseSpeed() {
        targetSpeed += acceleration;
    }

    void DecreaseSpeed() {
        targetSpeed -= acceleration;
    }

    void IncreaseHeading() {
        targetHeading += 20;
    }

    void DecreaseHeading() {
        targetHeading -= 20;
    }

    void Stop() {
        targetSpeed = 0;
    }
};

struct PhysicsComponent3D : public PhysicsComponent2D {
    using PhysicsComponent2D::PhysicsComponent2D;

    float speedY = 0;
    float targetSpeedY = 0;

    void tick(float dt) override {
        auto ref = object->GetComponent<TransformComponent>();
        if (!ref) return;
        auto& transform = ref->get();  

        velocity = CalculateVelocity(dt);
        transform.position = transform.position + velocity * dt;
        transform.rotation = raylib::Vector3(transform.rotation.x, heading, transform.rotation.z);
    }

    raylib::Vector3 CalculateVelocity(float dt){
        static constexpr auto AngleClamp = [](raylib::Degree angle) -> raylib::Degree {
		    float decimal = float(angle) - int(angle);
		    int whole = int(angle) % 360;
		    whole += (whole < 0) * 360;
		    return decimal + whole;
	    };

        targetHeading = AngleClamp(targetHeading);
        heading = AngleClamp(heading);

        float difference = abs(targetHeading - heading);
        if (targetHeading > heading) {
            if (difference < 180) heading += turningRate * dt;
            else if (difference > 180) heading -= turningRate * dt;
        }
        else if (targetHeading < heading) {
            if (difference > 180) heading += turningRate * dt;
            else if (difference < 180) heading -= turningRate * dt;
        }
        if(difference < .5) heading = targetHeading;

        if (targetSpeed > speed) speed += acceleration * dt;
        else if (targetSpeed < speed) speed -= acceleration * dt;

        if (targetSpeedY > speedY) speedY += acceleration * dt;
        else if (targetSpeedY < speedY) speedY -= acceleration * dt;

        return {speed * cos(heading.RadianValue()), speedY, -speed * sin(heading.RadianValue())};
    }

    void IncreaseSpeedY() {
        targetSpeedY += acceleration;
    }

    void DecreaseSpeedY() {
        targetSpeedY -= acceleration;
    }

    void Stop() {
        targetSpeed = 0;
        targetSpeedY = 0;
    }
};

struct InputComponent : public Component {
    using Component::Component;

    raylib::BufferedInput inputs;

    void setup() override {
        auto ref = object->GetComponent<PhysicsComponent2D>();
        if (!ref) return;
        auto& physics = ref->get();

        inputs["forward"] =
            raylib::Action::button_set( {raylib::Button::key(KEY_W), raylib::Button::key(KEY_UP)})
                .SetPressedCallback([&physics]{
                    physics.IncreaseSpeed();
            }).move();

        inputs["back"] =
            raylib::Action::button_set( {raylib::Button::key(KEY_S), raylib::Button::key(KEY_DOWN)})
                .SetPressedCallback([&physics]{
                    physics.DecreaseSpeed();
            }).move();

        inputs["upheading"] =
            raylib::Action::button_set( {raylib::Button::key(KEY_A), raylib::Button::key(KEY_LEFT)})
                .SetPressedCallback([&physics]{
                    physics.IncreaseHeading();
            }).move();

        inputs["backheading"] =
            raylib::Action::button_set( {raylib::Button::key(KEY_D), raylib::Button::key(KEY_RIGHT)})
                .SetPressedCallback([&physics]{
                    physics.DecreaseHeading();
            }).move();

        inputs["stop"] =
            raylib::Action::key(KEY_SPACE)
                .SetPressedCallback([&physics]{
                    physics.Stop();
            }).move();

        auto ref3D = object->GetComponent<PhysicsComponent3D>();
        if (!ref3D) return;
        auto& physics3D = ref3D->get();

        inputs["up"] =
            raylib::Action::key(KEY_Q)
                .SetPressedCallback([&physics3D]{
                    physics3D.IncreaseSpeedY();
            }).move();

        inputs["down"] =
            raylib::Action::key(KEY_E)
                .SetPressedCallback([&physics3D]{
                    physics3D.DecreaseSpeedY();
            }).move();

        inputs["stop"] =
            raylib::Action::key(KEY_SPACE)
                .SetPressedCallback([&physics3D]{
                    physics3D.Stop();
            }).move();
    }
};

///////////////////////////////////////////////////////////////////////////
// MAIN AREA //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

bool ProcessInput(Entity&);

int main() {
	// Create window
	const int screenWidth = 800;
	const int screenHeight = 450;
	raylib::Window window(screenWidth, screenHeight, "CS381 - Assignment 6");

    // Load ships
    std::vector<Entity> entities;
    Entity& ship1 = entities.emplace_back();
    ship1.AddComponent<RenderingComponent>(raylib::Model("../meshes/CargoG_HOSBrigadoon.glb"));
    ship1.GetComponent<TransformComponent>()->get().scale = raylib::Vector3(0.02, 0.02, 0.02);

    Entity& ship2 = entities.emplace_back();
    ship2.AddComponent<RenderingComponent>(raylib::Model("../meshes/Container_ShipLarge.glb"));
    ship2.GetComponent<TransformComponent>()->get().scale = raylib::Vector3(0.005, 0.005, 0.005);

    Entity& ship3 = entities.emplace_back();
    ship3.AddComponent<RenderingComponent>(raylib::Model("../meshes/ddg51.glb"));

    Entity& ship4 = entities.emplace_back();
    ship4.AddComponent<RenderingComponent>(raylib::Model("../meshes/OrientExplorer.glb"));
    ship4.GetComponent<TransformComponent>()->get().scale = raylib::Vector3(0.01, 0.01, 0.01);

    Entity& ship5 = entities.emplace_back();
    ship5.AddComponent<RenderingComponent>(raylib::Model("../meshes/SmitHouston_Tug.glb"));
    ship5.GetComponent<TransformComponent>()->get().scale = raylib::Vector3(1.2, 1.2, 1.2);

    raylib::Vector3 zRotation = {0,0,1};
    raylib::Vector3 xRotation = {1,0,0};
    float boatValues[5][6] = {
    // posX, posY, acc, turn, maxSpd, minSpd
        { 100,  100, 20, 30, 50, -50},  // brigadoon
        {-100, -100, 5 , 10, 40, -40}, // container
        { 100, -100, 10, 20, 30, -30},  // ddg
        {-100,  100, 15, 40, 20, -20},  // orient
        {   0,    0, 30, 50, 10, -10}        // tug
    };

    // Assign ship values
    int size = entities.size();
    for (int i = 0; i < size; i++){
        entities[i].GetComponent<TransformComponent>()->get().position = raylib::Vector3(boatValues[i][0], 0, boatValues[i][1]);
        entities[i].GetComponent<TransformComponent>()->get().rotation = raylib::Vector3(90, 0, 90);
        entities[i].AddComponent<PhysicsComponent2D>(boatValues[i][2], boatValues[i][3], boatValues[i][4], boatValues[i][5], zRotation);
        entities[i].AddComponent<InputComponent>();
        entities[i].GetComponent<InputComponent>()->get().setup();
    }
    ship5.GetComponent<TransformComponent>()->get().rotation = raylib::Vector3(0, 0, 0);
    ship5.GetComponent<PhysicsComponent2D>()->get().rotationAxis = raylib::Vector3(0,1,0);

    // Generate planes for each ship
    for (int i = 0; i < size; i++) {
        Entity& plane = entities.emplace_back();
        plane.AddComponent<RenderingComponent>(raylib::Model("../meshes/PolyPlane.glb"));
        plane.GetComponent<TransformComponent>()->get().position = raylib::Vector3(boatValues[i][0], 30, boatValues[i][1]);
        plane.AddComponent<PhysicsComponent3D>(50, 40, 20, -20); // planes can use same properties
        plane.AddComponent<InputComponent>();
        plane.GetComponent<InputComponent>()->get().setup();
    }

    int selectedVehicle = 0;
    entities[selectedVehicle].GetComponent<RenderingComponent>()->get().isBounded = true;

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

	// Create skybox
	cs381::SkyBox skybox("textures/skybox.png");

	// Create ocean
	auto mesh = raylib::Mesh::Plane(10000, 10000, 50, 50, 25);
	raylib::Model ocean = ((raylib::Mesh*)&mesh)->LoadModelFrom();
	raylib::Texture water("textures/water.jpg");
	water.SetFilter(TEXTURE_FILTER_BILINEAR);
	water.SetWrap(TEXTURE_WRAP_REPEAT);
	ocean.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = water;

	// Main loop
	bool keepRunning = true;
	while(!window.ShouldClose() && keepRunning) {
		// Updates
        if (IsKeyPressed(KEY_TAB)){
            entities[selectedVehicle].GetComponent<RenderingComponent>()->get().isBounded = false;
            selectedVehicle++;
            selectedVehicle %= entities.size();
            entities[selectedVehicle].GetComponent<RenderingComponent>()->get().isBounded = true;
        }

        entities[selectedVehicle].GetComponent<InputComponent>()->get().inputs.PollEvents();
		
		// Rendering
		{
			// Clear screen
			window.ClearBackground(BLACK);

			camera.BeginMode();
			{
				// Render skybox and ocean
				skybox.Draw();
				ocean.Draw({});

                for(Entity&e: entities) e.tick(window.GetFrameTime());

                // EXTRA CREDIT: Camera pans toward target (plane)
                camera.SetTarget(entities[selectedVehicle].GetComponent<TransformComponent>()->get().position);
			}
			camera.EndMode();

			// Measure our FPS
			DrawFPS(10, 10);
		}
		window.EndDrawing();
	}

	return 0;
}