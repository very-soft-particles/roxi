// =====================================================================================
//
//       Filename:  rx_ecs.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2025-01-08 12:08:13 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Robyn Gray (rg), ...
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "pods.hpp"
#include "rx_allocator.hpp"
#include "ecs_resources.hpp"
#include "rx_system.hpp"
#include "rx_vocab.h"
#include "../../../lofi/core/include/ecs/l_ecs.hpp"

#include "../gpu/data/data.hpp"

    
namespace roxi {
  namespace ecs {
    struct Position {
      f32 x;
      f32 y;
      f32 z;
    };

    using ComponentList = List
      < Position
      , AABB
      , Light
      , LightCell
      , AnimationNode
      , KeyPosition
      , KeyRotation
      , KeyScale
      , BoneInfo
      , InstanceData
      , MeshData
      , Vertex
      , ubo::Camera
      >;

    namespace tags {
      
        struct Renderable {};
      struct Animatable {};

    }		// -----  end of namespace tags  ----- 

    using TagList = List
      < tags::Renderable
      , tags::Animatable
      >;

    namespace archetypes {
      
        struct GraphicsScene {
          using type = List
            < tags::Renderable
            , MeshData
            , Position
            >;
          static constexpr u64 size = 64;
        };

      struct GraphicsLight { 
        using type = List
          < Light
          , Position
          >;
        static constexpr u64 size = 64;
      };

    }		// -----  end of namespace archetypes  ----- 

    using ArchetypeList = List
      < archetypes::GraphicsScene
      , archetypes::GraphicsLight
      >;

  }
}


using roxi_ecs_descriptor_t = lofi::ecs::ECSDescriptor<roxi::ecs::ComponentList, roxi::ecs::TagList, roxi::ecs::ArchetypeList>;

template<>
struct lofi::ecs::ECSSettings<roxi::RoxiECSConfigID> {
  using type = roxi_ecs_descriptor_t;
};
    
namespace roxi {
 
  namespace ecs {

    using RoxiStaticECS = lofi::ecs::Manager<lofi::ecs::Config<roxi::RoxiECSConfigID>>;

    class Manager {
    private:
      static RoxiStaticECS _static_ecs;

    public:
      static RoxiStaticECS* instance() {
        return &_static_ecs;
      }
    };

  }		// -----  end of namespace ecs  ----- 

}		// -----  end of namespace roxi  ----- 

#define RX_ECS() roxi::ecs::Manager::instance()
