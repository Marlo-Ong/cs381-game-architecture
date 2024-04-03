#include "raylib-cpp.hpp"
#include "skybox.hpp"
#include <concepts>
#include <memory>
#include <optional>
#include <iostream>
#include <BufferedInput.hpp>

template<typename T>
concept Transformer = requires(T t, raylib::Transform m) {
	{ t.operator()(m) } -> std::convertible_to<raylib::Transform>;
};

struct CalculateVelocityParams {
    static constexpr float acceleration = 5;
    static constexpr float angularAcceleration = 15;

    std::array<raylib::Vector3, 10> position;
    std::array<float, 10> targetSpeed;
    std::array<raylib::Degree, 10> targetHeading;
    std::array<float, 10> speed;
    std::array<raylib::Degree, 10> heading;
    float dt;

    std::array<float, 10> maxSpeed;
    std::array<float, 10> minSpeed;

};

using Entity = uint8_t;

bool ProcessInput(Entity&);
//bool ProcessInput(raylib::Degree& planeTargetHeading, float& planeTargetSpeed, size_t& selectedPlane);
raylib::Vector3 CaclulateVelocity(const CalculateVelocityParams& data);
void DrawBoundedModel(raylib::Model& model, Transformer auto transformer);
void DrawModel(raylib::Model& model, Transformer auto transformer);

// struct Component {
//     struct Entity* object;

//     Component(struct Entity& e) : object(&e) {}

//     virtual void setup() {};
//     virtual void cleanup() {};
//     virtual void tick(float dt) {};
// };

// struct TransformComponent : public Component {
//     using Component::Component; // using the same constructor as Component
//     raylib::Vector3 position = {0,0,0};
//     raylib::Vector3 rotation = {0,0,0};
//     raylib::Vector3 scale = {1, 1, 1};
// };

// struct Entity {
//     std::vector<std::unique_ptr<Component>> components;
//     // unique ptr tied to given component, deletes if component is deleted

//     Entity() { AddComponent<TransformComponent>(); }
//     Entity(const Entity&) = delete; // Compiler error if you try to copy an Entity, use the move constructor instead
//     Entity(Entity&& other) : components(std::move(other.components)) {
//         for (auto& c : components)
//             c->object = this;
//     }

//     template<std::derived_from<Component> T, typename... Ts> // derived_from<Component> -> only takes types that are children of Component class
//     size_t AddComponent(Ts... args) {
//         auto c = std::make_unique<T>(*this, std::forward<Ts>(args)...);
//         components.push_back(std::move(c));
//         return components.size() - 1;
//     }

//     template<std::derived_from<Component> T>
//     std::optional<std::reference_wrapper<T>> GetComponent() { // reference_wrapper allows & references to be placed within vectors, lists
//                                                               // std::optional same as [if (x), y = *x ]

//         // Optimization: since all Entities have a Transform, skip the loop below
//         if constexpr(std::is_same_v<T, TransformComponent>) { 
//             T* cast = dynamic_cast<T*>(components[0].get()); // components[0] = transform comp.
//             if(cast) return *cast;
//         }

//         // Loop through components and return the one requested
//         for(auto& c : components) {
//             T* cast = dynamic_cast<T*>(c.get());
//             if(cast) return *cast;
//         }

//         // Else return null
//         return std::nullopt; // null value when using "optional"
//     }

//     void tick(float dt) {
//         for(auto& c : components)
//             c->tick(dt);
//     }
// };

// struct RenderingComponent : public Component {
//     using Component::Component;
//     RenderingComponent(Entity& e, raylib::Model&& model) : Component(e), model(std::move(model)) {};

//     raylib::Model model; // assuming set on construction
//     bool isBounded = false;
    
//     void tick(float dt) override {
//         auto ref = object->GetComponent<TransformComponent>(); // try getting optional ref
//         if (!ref) return;                                      // if the component exists 
//         auto& transform = ref->get();                          // initialize reference

//         raylib::Transform backupTransform = model.transform;

//         model.transform = raylib::Transform(model.transform)
//             .Translate(transform.position)
//             .RotateXYZ(transform.rotation * DEG2RAD)
//             .Scale(transform.scale.x, transform.scale.y, transform.scale.z);
//         model.Draw({});

//         if (isBounded) model.GetTransformedBoundingBox().Draw();
//         model.transform = backupTransform;
//     }
// };

// struct PhysicsComponent2D : public Component {
//     using Component::Component;

//     PhysicsComponent2D(Entity& obj, float a, float b, float c, float d, raylib::Vector3 rot = {0,1,0}) : Component(obj) {
//         acceleration = a;
//         turningRate = b;
//         maxSpeed = c;
//         minSpeed = d;
//         rotationAxis = rot;

//         targetSpeed = 0;
//         speed = 0;
//         targetHeading = raylib::Degree(90);
//         heading = raylib::Degree(90);
//     };

//     float acceleration;
//     float turningRate;

//     float maxSpeed;
//     float minSpeed;
//     float targetSpeed;
//     float speed;
//     raylib::Vector3 velocity;
//     raylib::Vector3 rotationAxis;

//     raylib::Degree targetHeading;
//     raylib::Degree heading;

//     void tick(float dt) override {
//         auto ref = object->GetComponent<TransformComponent>();
//         if (!ref) return;
//         auto& transform = ref->get();  

//         velocity = CalculateVelocity(dt);
//         transform.position = transform.position + velocity * dt;
//         transform.rotation = raylib::Vector3(heading, heading, heading).Multiply(rotationAxis).Add(
//             transform.rotation.Multiply(raylib::Vector3::One().Subtract(rotationAxis)));
//         std::cout <<"(" << transform.rotation.x << ", " << transform.rotation.y << ", " << transform.rotation.z << ") - " <<"(" << transform.position.x << ", " << transform.position.y << ", " << transform.position.z << ")" << std::endl;
//     }

//     raylib::Vector3 CalculateVelocity(float dt){
//         static constexpr auto AngleClamp = [](raylib::Degree angle) -> raylib::Degree {
// 		    float decimal = float(angle) - int(angle);
// 		    int whole = int(angle) % 360;
// 		    whole += (whole < 0) * 360;
// 		    return decimal + whole;
// 	    };

//         float tempTarget = AngleClamp(targetHeading);
//         float tempHead = AngleClamp(heading);

//         float difference = abs(tempTarget - tempHead);
//         if (tempTarget > tempHead) {
//             if (difference < 180) heading += turningRate * dt;
//             else if (difference > 180) heading -= turningRate * dt;
//         }
//         else if (tempTarget < tempHead) {
//             if (difference > 180) heading += turningRate * dt;
//             else if (difference < 180) heading -= turningRate * dt;
//         }
//         if(difference < 2) heading = targetHeading;

//         if (targetSpeed > speed) speed += acceleration * dt;
//         else if (targetSpeed < speed) speed -= acceleration * dt;

//         return {speed * sin(heading.RadianValue()), 0, -speed * cos(heading.RadianValue())};
//     }

//     void IncreaseSpeed() {
//         targetSpeed += acceleration;
//     }

//     void DecreaseSpeed() {
//         targetSpeed -= acceleration;
//     }

//     void IncreaseHeading() {
//         targetHeading += 20;
//     }

//     void DecreaseHeading() {
//         targetHeading -= 20;
//     }

//     void Stop() {
//         targetSpeed = 0;
//     }
// };

// struct PhysicsComponent3D : public PhysicsComponent2D {
//     using PhysicsComponent2D::PhysicsComponent2D;

//     float speedY = 0;
//     float targetSpeedY = 0;

//     void tick(float dt) override {
//         auto ref = object->GetComponent<TransformComponent>();
//         if (!ref) return;
//         auto& transform = ref->get();  

//         velocity = CalculateVelocity(dt);
//         transform.position = transform.position + velocity * dt;
//         transform.rotation = raylib::Vector3(transform.rotation.x, heading, transform.rotation.z);
//     }

//     raylib::Vector3 CalculateVelocity(float dt){
//         static constexpr auto AngleClamp = [](raylib::Degree angle) -> raylib::Degree {
// 		    float decimal = float(angle) - int(angle);
// 		    int whole = int(angle) % 360;
// 		    whole += (whole < 0) * 360;
// 		    return decimal + whole;
// 	    };

//         targetHeading = AngleClamp(targetHeading);
//         heading = AngleClamp(heading);

//         float difference = abs(targetHeading - heading);
//         if (targetHeading > heading) {
//             if (difference < 180) heading += turningRate * dt;
//             else if (difference > 180) heading -= turningRate * dt;
//         }
//         else if (targetHeading < heading) {
//             if (difference > 180) heading += turningRate * dt;
//             else if (difference < 180) heading -= turningRate * dt;
//         }
//         if(difference < .5) heading = targetHeading;

//         if (targetSpeed > speed) speed += acceleration * dt;
//         else if (targetSpeed < speed) speed -= acceleration * dt;

//         if (targetSpeedY > speedY) speedY += acceleration * dt;
//         else if (targetSpeedY < speedY) speedY -= acceleration * dt;

//         return {speed * cos(heading.RadianValue()), speedY, -speed * sin(heading.RadianValue())};
//     }

//     void IncreaseSpeedY() {
//         targetSpeedY += acceleration;
//     }

//     void DecreaseSpeedY() {
//         targetSpeedY -= acceleration;
//     }

//     void Stop() {
//         targetSpeed = 0;
//         targetSpeedY = 0;
//     }
// };

// struct InputComponent : public Component {
//     using Component::Component;

//     raylib::BufferedInput inputs;

//     void setup() override {
//         auto ref = object->GetComponent<PhysicsComponent2D>();
//         if (!ref) return;
//         auto& physics = ref->get();

//         inputs["forward"] =
//             raylib::Action::button_set( {raylib::Button::key(KEY_W), raylib::Button::key(KEY_UP)})
//                 .SetPressedCallback([&physics]{
//                     physics.IncreaseSpeed();
//             }).move();

//         inputs["back"] =
//             raylib::Action::button_set( {raylib::Button::key(KEY_S), raylib::Button::key(KEY_DOWN)})
//                 .SetPressedCallback([&physics]{
//                     physics.DecreaseSpeed();
//             }).move();

//         inputs["upheading"] =
//             raylib::Action::button_set( {raylib::Button::key(KEY_A), raylib::Button::key(KEY_LEFT)})
//                 .SetPressedCallback([&physics]{
//                     physics.IncreaseHeading();
//             }).move();

//         inputs["backheading"] =
//             raylib::Action::button_set( {raylib::Button::key(KEY_D), raylib::Button::key(KEY_RIGHT)})
//                 .SetPressedCallback([&physics]{
//                     physics.DecreaseHeading();
//             }).move();

//         inputs["stop"] =
//             raylib::Action::key(KEY_SPACE)
//                 .SetPressedCallback([&physics]{
//                     physics.Stop();
//             }).move();

//         auto ref3D = object->GetComponent<PhysicsComponent3D>();
//         if (!ref3D) return;
//         auto& physics3D = ref3D->get();

//         inputs["up"] =
//             raylib::Action::key(KEY_Q)
//                 .SetPressedCallback([&physics3D]{
//                     physics3D.IncreaseSpeedY();
//             }).move();

//         inputs["down"] =
//             raylib::Action::key(KEY_E)
//                 .SetPressedCallback([&physics3D]{
//                     physics3D.DecreaseSpeedY();
//             }).move();

//         inputs["stop"] =
//             raylib::Action::key(KEY_SPACE)
//                 .SetPressedCallback([&physics3D]{
//                     physics3D.Stop();
//             }).move();
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
            data.size() / elementSize
        };
    }

    template<typename Tcomponent>
    Tcomponent& GetOrAllocate(Entity e) {
        assert(sizeof(Tcomponent) == elementSize);
        size_t size = data.size() / elementSize; // number of elements (as components, not bytes)

        if (size <= e)
            Allocate<Tcomponent>(std::max<int64_t>(int64_t(e) - size, 1));
        return Get<Tcomponent>(e);
    }
};

struct SkiplistComponentStorage {
    size_t elementSize = -1;
    std::vector<uint16_t> indices;
    std::vector<std::byte> data;

    // Constructors: default, give size, give type
    SkiplistComponentStorage() : elementSize(-1), data(1, std::byte{0}) {}
    SkiplistComponentStorage(size_t elementSize) : elementSize(elementSize) {data.resize(5 * elementSize);}

    template<typename Tcomponent>
    SkiplistComponentStorage(Tcomponent reference = {}) : SkiplistComponentStorage(sizeof(Tcomponent)) {}
    // reference = {} calls default constructor of component

    template<typename Tcomponent>
    Tcomponent& Get(Entity e) {
        assert(sizeof(Tcomponent) == elementSize);  // components must be the same size
        assert(e < indices.size());
        assert(indices[e] != std::numeric_limits<uint16_t>::max());
        return *(Tcomponent*)(data.data() + indices[e]);
    }

    private:
        template<typename Tcomponent>
        std::pair<Tcomponent&, Entity> Allocate() {
            assert(sizeof(Tcomponent) == elementSize);
            data.insert(data.end(), elementSize, std::byte{0}); // at the end of the vector, allocate data equal to what we want
            return {
                *new(data.data() + data.size() - elementSize) Tcomponent(),
                data.size() / elementSize
            };
        }

    public:
        template<typename Tcomponent>
        Tcomponent& Allocate(Entity e) {
            auto [ret, i] = Allocate<Tcomponent>();
            indices[e] = i * elementSize;
            return ret;
        }

        template<typename Tcomponent>
        Tcomponent& GetOrAllocate(Entity e) {
            assert(sizeof(Tcomponent) == elementSize);
            if(indices.size() <= e)
                indices.insert(indices.end(), std::max<int64_t>(int64_t(e) - indices.size(), 1), -1);
            if(indices[e] == std::numeric_limits<uint16_t>::max())
                return Allocate<Tcomponent>(e);
            return Get<Tcomponent>(e);
        }
};

extern size_t globalComponentCounter; // declared in counter.cpp
template<typename T>
size_t GetComponentID(T reference = {}) {
    static size_t id = globalComponentCounter++;
    return id;
}

struct Scene {
    std::vector<std::vector<bool>> entityMasks;
    std::vector<ComponentStorage> storages = {ComponentStorage()};

    template<typename Tcomponent>
    ComponentStorage& GetStorage() {
        size_t id = GetComponentID<Tcomponent>();
        if(storages.size() <= id)
            storages.insert(storages.cend(), std::max<int64_t>(id - storages.size(), 1), ComponentStorage());
        if(storages[id].elementSize == std::numeric_limits<size_t>::max())
            storages[id] = ComponentStorage(Tcomponent{});
        return storages[id];
    }

    Entity CreateEntity() {
        Entity e = entityMasks.size();
        entityMasks.emplace_back(std::vector<bool>{false});
        return e;
    }

    template<typename Tcomponent>
    Tcomponent& AddComponent(Entity e) {
        size_t id = GetComponentID<Tcomponent>();
        auto& eMask = entityMasks[e];
        if(eMask.size() <= id)
            eMask.resize(id+1, false);
        eMask[id] = true;
        return GetStorage<Tcomponent>().template GetOrAllocate<Tcomponent>(e);
    }

    // Implement RemoveComponent

    template<typename Tcomponent>
    Tcomponent& GetComponent(Entity e) {
        size_t id = GetComponentID<Tcomponent>();
        assert(entityMasks[e][id]);
        return GetStorage<Tcomponent>().template Get<Tcomponent>(e);
    }   

    template<typename Tcomponent>
    bool HasComponent(Entity e) {
        size_t id = GetComponentID<Tcomponent>();
        return entityMasks.size() > e && entityMasks[e].size() > id && entityMasks[e][id];
    }    
};

struct Rendering {
    raylib::Model* model;
    bool drawBoundingBox = false;
};

struct Physics {};

void DrawSystem(Scene& scene) {
    for(Entity e = 0; e < scene.entityMasks.size(); e++) {
        if(!scene.HasComponent<Rendering>(e)) continue;
        if(!scene.HasComponent<Physics>(e)) continue;
        auto& rendering = scene.GetComponent<Rendering>(e);

        auto transformer = [](raylib::Transform t) -> raylib::Transform {
            return t;
        };

        if (rendering.drawBoundingBox)
            DrawBoundedModel(*rendering.model, transformer);
        else DrawModel(*rendering.model, transformer);
    }
}


///////////////////////////////////////////////////////////////////////////
// MAIN AREA //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

int main() {
	// Create window
	const int screenWidth = 800;
	const int screenHeight = 450;
	raylib::Window window(screenWidth, screenHeight, "CS381 - Assignment 8");

    // APRIL 1
    Scene scene;
    auto e = scene.CreateEntity();
    raylib::Model plane("meshes/PolyPlane.glb");
    scene.AddComponent<Rendering>(e) = {&plane, false};

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

		// Rendering
		{
			// Clear screen
			window.ClearBackground(BLACK);

			camera.BeginMode();
			{
				// Render skybox and ocean
				skybox.Draw();
				ocean.Draw({});

                DrawSystem(scene);
			}
			camera.EndMode();

			// Measure our FPS
			DrawFPS(10, 10);
		}
		window.EndDrawing();
	}

	return 0;
}

// raylib::Vector3 CaclulateVelocity(const CalculateVelocityParams& data) {
// 	static constexpr auto AngleClamp = [](raylib::Degree angle) -> raylib::Degree {
// 		float decimal = float(angle) - int(angle);
// 		int whole = int(angle) % 360;
// 		whole += (whole < 0) * 360;
// 		return decimal + whole;
// 	};

// 	float target = Clamp(data.targetSpeed, data.minSpeed, data.maxSpeed);
// 	if(data.speed < target) data.speed += data.acceleration * data.dt;
// 	else if(data.speed > target) data.speed -= data.acceleration * data.dt;
// 	data.speed = Clamp(data.speed, data.minSpeed, data.maxSpeed);

// 	target = AngleClamp(data.targetHeading);
// 	float difference = abs(target - data.heading);
// 	if(target > data.heading) {
// 		if(difference < 180) data.heading += data.angularAcceleration * data.dt;
// 		else if(difference > 180) data.heading -= data.angularAcceleration * data.dt;
// 	} else if(target < data.heading) {
// 		if(difference < 180) data.heading -= data.angularAcceleration * data.dt;
// 		else if(difference > 180) data.heading += data.angularAcceleration * data.dt;
// 	} 
// 	if(difference < .5) data.heading = target; // If the heading is really close to correct 
// 	data.heading = AngleClamp(data.heading);
// 	raylib::Radian angle = raylib::Degree(data.heading);

// 	return {cos(angle) * data.speed, 0, -sin(angle) * data.speed};
// }

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