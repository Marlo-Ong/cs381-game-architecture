#include <raylib-cpp.hpp>

#include <memory>
#include <ranges>
#include <iostream>
#include <cassert>
#include <concepts>
#include <optional>

// #include "inputs.hpp"
#include "skybox.hpp"

template<typename T>
concept Transformer = requires(T t, raylib::Transform m) {
	{ t.operator()(m) } -> std::convertible_to<raylib::Transform>;
};

struct CalculateVelocityParams {
	static constexpr float acceleration = 5;
	static constexpr float angularAcceleration = 15;

	float targetSpeed;
	raylib::Degree targetHeading;
	float& speed;
	raylib::Degree& heading;
	float dt;

	float maxSpeed = 50;
	float minSpeed = 0;
};

// struct Component {
//     Entity* object;

//     Component(struct Entity& e) : object(&e) {}

//     virtual void setup() = 0;
//     virtual void cleaup() = 0;
//     virtual void tick(float dt) = 0;
// };

// struct TransformComponent : public Component {
//     using Component::Component;
//     raylib::Vector3 position = {0,0,0};
//     raylib::Quaternion rotation = raylib::Quaternion::Identity();
// };

// struct Entity {
//     std::vector<std::unique_ptr<Component*>> components;
//     // unique ptr tied to given component, deletes if component is deleted

//     Entity() { AddComponent<TransformComponent>(); }
//     Entity(const Entity&) = delete;
//     Entity(Entity&& other) : components(std::move(other.components)) {
//         for (auto& c : components)
//             c->object = this;
//     }

//     template<std::derived_from<Component> T, typename... Ts>
//     size_t AddComponent(Ts... args) {
//         auto c = std::make_unique<T>(*this, std::forward<Ts>(args)...);
//         components.push_back(std::move(c));
//         return components.size() - 1;
//     }

//     template<std::derived_from<Component> T>
//     std::optional<std::reference_wrapper<T>> GetComponent() {
//         if constexpr(std::is_same_v<T, TransformComponent>) {
//             T* cast = dynamic_cast<T>(components[0].get());
//             if(cast) return *cast;
//         }

//         for(auto& c : components) {
//             T* cast = dynamic_cast<T>(c.get());
//             if(cast) return *cast;
//         }

//         return std::nullopt;
//     }

//     void tick(float dt) {
//         for(auto& c : components)
//             c->tick(dt);
//     }
// };

// struct RenderingComponent : public Component {
//     raylib::Model model; // assuming set on construction

//     void tick(float dt) override {
//         auto ref = object->GetComponent<TransformComponent>(); // try getting optional ref
//         if (!ref) return;                                      // if the component exists 
//         auto& transform = ref->get();                          // initialize reference

//         transform.position;
//         auto [axis, angle] = transform.rotation.ToAxisAngle(); // C++ unpacking, like (Python, tuples)
        
//         model.Draw(transform.position, axis, angle);
//     }
// };

// Can only store one type of thing
struct ComponentStorage {
    size_t elementSize = -1;
    std::vector<std::byte> data;

    // Constructors: default, give size, give type
    ComponentStorage() : elementSize(-1), data(1, std::byte(0)) {}
    ComponentStorage(size_t elementSize) : elementSize(elementSize) {data.resize(5 * elementSize);}

    template<typename Tcomponent>
    ComponentStorage(Tcomponent reference = {}) : ComponentStorage(sizeof(Tcomponent)) {}
    // reference = {} calls default constructor of component

    template<typename Tcomponent>
    Tcomponent& Get(Entity e) {
        assert(sizeof(Tcomponent) == elementSize);  // components must be the same size
        assert(e < (data.size() / elementSize));
        return *(Tcomponent*)(data.data() + e * elementSize);
    }

    template<typename Tcomponent>
    std::pair<Tcomponent&, Entity> Allocate(size_t count = 1) {
        assert(sizeof(Tcomponent) == elementSize);
        assert(count < 255);

        auto originalEnd = data.size();
        data.insert(data.end(), elementSize * count, std::byte{0}); // at the end of the vector, allocate data equal to what we want

        for(size_t i = 0; i < count - 1; i++)
            new(data.data() + originalEnd + i * elementSize) Tcomponent(); // "placement new": call new() with an array pointer, allocates thing to the array element
        return {
            *new(data.data() + data.size() - elementSize) Tcomponent(),
            data.size() / elementSize;
        }
    }

    template<typename Tcomponent>
    Tcomponent& GetOrAllocate(Entity e) {
        assert(sizeof(Tcomponent) == elementSize);
        size_t size = data.size() / elementSize; // number of elements (as components, not bytes)

        if (size <= e)
            Allocate<Tcomponent>(std::max<int64_t>(int64_t(e) - size, 1));
        return Get<Tcomponent>(e);
};

bool ProcessInput(raylib::Degree& planeTargetHeading, float& planeTargetSpeed, size_t& selectedPlane);
raylib::Vector3 CaclulateVelocity(const CalculateVelocityParams& data);
void DrawBoundedModel(raylib::Model& model, Transformer auto transformer);
void DrawModel(raylib::Model& model, Transformer auto transformer);


int main() {
	// Create window
	const int screenWidth = 800 * 2;
	const int screenHeight = 450 * 2;
	raylib::Window window(screenWidth, screenHeight, "CS381 - Assignment 3");
	// cs381::Inputs inputs(window);

	// Create camera
	auto camera = raylib::Camera(
		raylib::Vector3(0, 120, -500), // Position
		raylib::Vector3(0, 0, 300), // Target
		raylib::Vector3::Up(), // Up direction
		45.0f,
		CAMERA_PERSPECTIVE
	);

	// Create skybox
	cs381::SkyBox skybox("textures/skybox.png");

	// Create ground
	auto mesh = raylib::Mesh::Plane(10000, 10000, 50, 50, 25);
	raylib::Model ground = ((raylib::Mesh*)&mesh)->LoadModelFrom();
	raylib::Texture grass("textures/grass.jpg");
	grass.SetFilter(TEXTURE_FILTER_BILINEAR);
	grass.SetWrap(TEXTURE_WRAP_REPEAT);
	ground.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = grass;

	// Create airplane
	raylib::Model plane("meshes/PolyPlane.glb");
	// Set plane0 variables
	raylib::Vector3 plane0Position = raylib::Vector3::Zero(), plane0Velocity = raylib::Vector3::Zero();
	float plane0TargetSpeed = 0, plane0Speed = 0;
	raylib::Degree plane0TargetHeading = 0, plane0Heading = 0;
	// Set plane1 variables
	raylib::Vector3 plane1Position = {50, 0, 0}, plane1Velocity = raylib::Vector3::Zero();
	float plane1TargetSpeed = 0, plane1Speed = 0;
	raylib::Degree plane1TargetHeading = 0, plane1Heading = 0;
	// Set plane2 variables
	raylib::Vector3 plane2Position = {-50, 0, 0}, plane2Velocity = raylib::Vector3::Zero();
	float plane2TargetSpeed = 0, plane2Speed = 0;
	raylib::Degree plane2TargetHeading = 0, plane2Heading = 0;

	size_t selectedPlane = 0;


	// Main loop
	bool keepRunning = true;
	while(!window.ShouldClose() && keepRunning) {
		// Updates
		// Process input for the selected plane
		switch(selectedPlane) {
			break; case 0: keepRunning = ProcessInput(plane0TargetHeading, plane0TargetSpeed, selectedPlane); 
			break; case 1: keepRunning = ProcessInput(plane1TargetHeading, plane1TargetSpeed, selectedPlane); 
			break; case 2: keepRunning = ProcessInput(plane2TargetHeading, plane2TargetSpeed, selectedPlane); 
		}
		
		// Apply simple physics to plane0
		plane0Velocity = CaclulateVelocity({
			.targetSpeed = plane0TargetSpeed,
			.targetHeading = plane0TargetHeading,
			.speed = plane0Speed,
			.heading = plane0Heading,
			.dt = window.GetFrameTime()
		});
		plane0Position = plane0Position + plane0Velocity * window.GetFrameTime();
		auto plane0Transformer = [plane0Position, plane0Heading](raylib::Transform transform) {
			return transform.Translate(plane0Position).RotateY(raylib::Degree(plane0Heading));
		};

		// Apply simple physics to plane1
		plane1Velocity = CaclulateVelocity(CalculateVelocityParams{
			.targetSpeed = plane1TargetSpeed,
			.targetHeading = plane1TargetHeading,
			.speed = plane1Speed,
			.heading = plane1Heading,
			.dt = window.GetFrameTime()
		});
		plane1Position = plane1Position + plane1Velocity * window.GetFrameTime();
		auto plane1Transformer = [plane1Position, plane1Heading](raylib::Transform transform) {
			return transform.Translate(plane1Position).RotateY(raylib::Degree(plane1Heading));
		};

		// Apply simple physics to plane2
		plane2Velocity = CaclulateVelocity(CalculateVelocityParams{
			.targetSpeed = plane2TargetSpeed,
			.targetHeading = plane2TargetHeading,
			.speed = plane2Speed,
			.heading = plane2Heading,
			.dt = window.GetFrameTime()
		});
		plane2Position = plane2Position + plane2Velocity * window.GetFrameTime();
		auto plane2Transformer = [plane2Position, plane2Heading](raylib::Transform transform) {
			return transform.Translate(plane2Position).RotateY(raylib::Degree(plane2Heading));
		};


		// Rendering
		window.BeginDrawing();
		{
			// Clear screen
			window.ClearBackground(BLACK);

			camera.BeginMode();
			{
				// Render skybox and ground
				skybox.Draw();
				ground.Draw({});

				// Draw the planes with a bounding box around the selected plane
				switch(selectedPlane) {
					break; case 0: {
						DrawBoundedModel(plane, plane0Transformer);
						DrawModel(plane, plane1Transformer);
						DrawModel(plane, plane2Transformer);
					} break; case 1: {
						DrawModel(plane, plane0Transformer);
						DrawBoundedModel(plane, plane1Transformer);
						DrawModel(plane, plane2Transformer);
					} break; case 2: {
						DrawModel(plane, plane0Transformer);
						DrawModel(plane, plane1Transformer);
						DrawBoundedModel(plane, plane2Transformer);
					}
				}
			}
			camera.EndMode();

			// Measure our FPS
			DrawFPS(10, 10);
		}
		window.EndDrawing();
	}

	return 0;
}

// Input handling
bool ProcessInput(raylib::Degree& planeTargetHeading, float& planeTargetSpeed, size_t& selectedPlane) {
	static bool wPressedLastFrame = false, sPressedLastFrame = false;
	static bool aPressedLastFrame = false, dPressedLastFrame = false;
	static bool tabPressedLastFrame = false;

	// If we hit escape... shutdown
	if(IsKeyDown(KEY_ESCAPE))
		return false;

	// WASD updates plane velocity
	if(IsKeyDown(KEY_W) && !wPressedLastFrame)
		planeTargetSpeed += 1;
	if(IsKeyDown(KEY_S) && !sPressedLastFrame)
		planeTargetSpeed -= 1;
	if(IsKeyDown(KEY_A) && !aPressedLastFrame)
		planeTargetHeading += 5;
	if(IsKeyDown(KEY_D) && !dPressedLastFrame)
		planeTargetHeading -= 5;  

	// Space sets velocity to 0!
	if(IsKeyDown(KEY_SPACE))
		planeTargetSpeed = 0;

	// Tab selects the next plane
	if(IsKeyDown(KEY_TAB) && !tabPressedLastFrame)
		selectedPlane = (selectedPlane + 1) % 3;

	// Save the state of the key for next frame
	wPressedLastFrame = IsKeyDown(KEY_W);
	sPressedLastFrame = IsKeyDown(KEY_S);
	aPressedLastFrame = IsKeyDown(KEY_A);
	dPressedLastFrame = IsKeyDown(KEY_D);

	tabPressedLastFrame = IsKeyDown(KEY_TAB);

	return true;
}

raylib::Vector3 CaclulateVelocity(const CalculateVelocityParams& data) {
	static constexpr auto AngleClamp = [](raylib::Degree angle) -> raylib::Degree {
		float decimal = float(angle) - int(angle);
		int whole = int(angle) % 360;
		whole += (whole < 0) * 360;
		return decimal + whole;
	};

	float target = Clamp(data.targetSpeed, data.minSpeed, data.maxSpeed);
	if(data.speed < target) data.speed += data.acceleration * data.dt;
	else if(data.speed > target) data.speed -= data.acceleration * data.dt;
	data.speed = Clamp(data.speed, data.minSpeed, data.maxSpeed);

	target = AngleClamp(data.targetHeading);
	float difference = abs(target - data.heading);
	if(target > data.heading) {
		if(difference < 180) data.heading += data.angularAcceleration * data.dt;
		else if(difference > 180) data.heading -= data.angularAcceleration * data.dt;
	} else if(target < data.heading) {
		if(difference < 180) data.heading -= data.angularAcceleration * data.dt;
		else if(difference > 180) data.heading += data.angularAcceleration * data.dt;
	} 
	if(difference < .5) data.heading = target; // If the heading is really close to correct 
	data.heading = AngleClamp(data.heading);
	raylib::Radian angle = raylib::Degree(data.heading);

	return {cos(angle) * data.speed, 0, -sin(angle) * data.speed};
}

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