#ifndef ECS_HPP
#define ECS_HPP

#include <memory>
#include <concepts>
#include <vector>
#include <deque>
#include <iostream>
#include <ranges>
#include <bitset>
#include <span>
#include <variant>
#include <cassert>

extern size_t globalComponentCounter;
namespace cs381 {
	template<typename T>
	size_t GetComponentID(T reference = {}) {
		static size_t id = globalComponentCounter++;
		return id;
	}

	using Entity = uint8_t; 

	struct ComponentStorage {
		size_t elementSize = -1;
		std::vector<std::byte> data;

		ComponentStorage() : elementSize(-1), data(1, std::byte{0}) {}
		ComponentStorage(size_t elementSize) : elementSize(elementSize) { data.reserve(10 * elementSize); }
		
		template<typename Tcomponent>
		ComponentStorage(Tcomponent reference = {}) : ComponentStorage(sizeof(Tcomponent)) {}

		template<typename Tcomponent>
		Tcomponent& Get(Entity e) {
			assert(sizeof(Tcomponent) == elementSize);
			assert(e <= (data.size() / elementSize));
			return *(Tcomponent*)(data.data() + e * elementSize);
		}

		template<typename Tcomponent>
		std::pair<Tcomponent&, size_t> Allocate(size_t count = 1) {
			assert(sizeof(Tcomponent) == elementSize);
			assert(count < 100);
			auto originalEnd = data.size();
			data.insert(data.end(), elementSize * count, std::byte{0});
			for(size_t i = 0; i < count - 1; i++) // Skip the last one
				new(data.data() + originalEnd + i * elementSize) Tcomponent();
			return {
				*new(data.data() + data.size() - elementSize) Tcomponent(),
				data.size() / elementSize
			};
		}

		template<typename Tcomponent>
		Tcomponent& GetOrAllocate(Entity e) {
			assert(sizeof(Tcomponent) == elementSize);
			size_t size = data.size() / elementSize;
			if (size <= e)
				Allocate<Tcomponent>(std::max<int64_t>(int64_t(e) - size, 1));
			return Get<Tcomponent>(e);
		}
	};


	template<typename Storage = ComponentStorage>
	struct Scene {
		std::vector<std::vector<bool>> entityMasks;
		std::vector<Storage> storages = {Storage()};

		template<typename Tcomponent>
		Storage& GetStorage() {
			size_t id = GetComponentID<Tcomponent>();
			if(storages.size() <= id)
				storages.insert(storages.cend(), std::max<int64_t>(id - storages.size(), 1), Storage());
			if (storages[id].elementSize == std::numeric_limits<size_t>::max())
				storages[id] = Storage(Tcomponent{});
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
				eMask.resize(id + 1, false);
			eMask[id] = true;
			return GetStorage<Tcomponent>().template GetOrAllocate<Tcomponent>(e);
		}

		template<typename Tcomponent>
		void RemoveComponent(Entity e) {
			size_t id = GetComponentID<Tcomponent>();
			auto& eMask = entityMasks[e];
			if(eMask.size() > id)
				eMask[id] = false;
		}

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




	// Niceties!

	struct SkiplistComponentStorage {
		size_t elementSize = -1;
		std::vector<size_t> indecies;
		std::vector<std::byte> data;

		SkiplistComponentStorage() : elementSize(-1), indecies(1, -1), data(1, std::byte{0}) {}
		SkiplistComponentStorage(size_t elementSize) : elementSize(elementSize) { data.reserve(5 * elementSize); }
		
		template<typename Tcomponent>
		SkiplistComponentStorage(Tcomponent reference = {}) : SkiplistComponentStorage(sizeof(Tcomponent)) {}

		template<typename Tcomponent>
		Tcomponent& Get(Entity e) {
			assert(sizeof(Tcomponent) == elementSize);
			assert(e < indecies.size());
			assert(indecies[e] != std::numeric_limits<size_t>::max());
			return *(Tcomponent*)(data.data() + indecies[e]);
		}

		template<typename Tcomponent>
		std::pair<Tcomponent&, size_t> Allocate() {
			assert(sizeof(Tcomponent) == elementSize);
			data.insert(data.end(), elementSize, std::byte{0});
			return {
				*new(data.data() + data.size() - elementSize) Tcomponent(),
				(data.size() - 1) / elementSize
			};
		}

		template<typename Tcomponent>
		Tcomponent& Allocate(Entity e) {
			auto [ret, i] = Allocate<Tcomponent>();
			indecies[e] = i * elementSize;
			return ret;
		}

		template<typename Tcomponent>
		Tcomponent& GetOrAllocate(Entity e) {
			assert(sizeof(Tcomponent) == elementSize);
			if (indecies.size() <= e)
				indecies.insert(indecies.end(), std::max<int64_t>(int64_t(e) - indecies.size(), 1), -1);
			if (indecies[e] == std::numeric_limits<size_t>::max())
				return Allocate<Tcomponent>(e);
			return Get<Tcomponent>(e);
		}
	};


	using post_increment_t = int;

	template<typename... Tcomponents>
	struct SceneView {
		Scene<ComponentStorage>& scene;

		struct Sentinel {};
		struct Iterator {
			Scene<ComponentStorage>* scene = nullptr;
			Entity e;

			bool valid() { return (scene->HasComponent<Tcomponents>(e) && ...); }

			bool operator==(Sentinel) { return scene == nullptr || e >= scene->entityMasks.size(); }

			Iterator& operator++(post_increment_t) { 
				do {
					e++;
				} while(!valid() && e < scene->entityMasks.size());
				return *this;
			}
			Iterator operator++() {
				Iterator old;
				operator++(0);
				return old;
			}

			// Entity operator*() { return e; }
			std::tuple<std::add_lvalue_reference_t<Tcomponents>...> operator*() { return { scene->GetComponent<Tcomponents>(e)... }; }
		};

		Iterator begin() { 
			Iterator out{&scene, 0}; 
			if(!out.valid()) ++out;
			return out;
		}
		Sentinel end() { return {}; }
	};
}

#endif // ECS_HPP
