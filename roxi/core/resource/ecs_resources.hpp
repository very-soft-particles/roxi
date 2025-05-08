#pragma once
#include "rx_vocab.h"
#include "rx_tuple.hpp"
#include "rx_container.hpp"

namespace roxi {
	namespace config {
		using u8 = uint8_t;
		using MeshID = u32;
		template<typename T, u64 Size>
		using Array = lofi::mem::ArrayContainerPolicy<T, Size, 8, lofi::mem::StackAllocPolicy>;
		using BoundingBox = lofi::tuple<float, float, float>;
		enum class MyEnum {
			This,
			is,
			my,
			enumerator
		};
		enum class Resolution {
			Low,
			Mid,
			High
		};

		namespace components {
			struct Position {
				u64 x;
				u64 y;
				u64 z;
			};
			struct Velocity {
				u64 x;
				u64 y;
				u64 z;
			};
			struct Acceleration {
				u64 x;
				u64 y;
				u64 z;
			};
			struct Mesh {
				MeshID id;
				Resolution res;
			};
			struct RenderableTag {};
			struct StaticCollisionTag {};
			struct DynamicCollisionTag {};
			struct Collision {
				BoundingBox bounding_box;
			};
			struct Camera {
				f32 frustum_depth;
			};
		} // -----  end of namespace components  ----- 

		using ComponentList = List<
				components::Position, components::Velocity, components::Acceleration, components::Mesh, components::Collision, 
			components::Camera
		>;

		using TagList = List<
			components::RenderableTag, components::StaticCollisionTag, components::DynamicCollisionTag
		>;

		namespace archetypes {
			struct StaticRenderables {
				using type = List<
					components::Position, components::Mesh, components::Collision
				>;
				static constexpr u64 size = 32;
			};

			struct DynamicRenderables {
				using type = List<
					components::Position, components::Velocity, components::Acceleration, components::Mesh, components::Collision
				>;
				static constexpr u64 size = 32;
			};

			struct StaticObject {
				using type = List<
					components::Position, components::Collision
				>;
				static constexpr u64 size = 32;
			};

			struct DynamicObject {
				using type = List<
					components::Position, components::Velocity, components::Acceleration, components::Collision
				>;
				static constexpr u64 size = 32;
			};

			struct Player {
				using type = List<
					components::Position, components::Velocity, components::Acceleration, components::Collision, components::Camera
				>;
				static constexpr u64 size = 4;
			};

			struct PlayerWMesh {
				using type = List<
					components::Position, components::Velocity, components::Acceleration, components::Mesh, components::Collision, components::Camera
				>;
				static constexpr u64 size = 4;
			};

		} // -----  end of namespace archetypes  ----- 

		using ArchetypeList = List<
				archetypes::StaticRenderables, archetypes::DynamicRenderables, archetypes::StaticObject, archetypes::DynamicObject, archetypes::Player, archetypes::PlayerWMesh
		>;

	} // -----  end of namespace config  ----- 

} // -----  end of namespace roxi  ----- 
