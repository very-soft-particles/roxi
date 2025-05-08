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
#include "rx_allocator.hpp"
#include "ecs_resources.hpp"
#include "rx_system.hpp"
#include "rx_vocab.h"

using roxi_ecs_descriptor_t = lofi::ecs::ECSDescriptor<roxi::config::ComponentList, roxi::config::TagList, roxi::config::ArchetypeList>;

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

      Array<System> _systems;

    public:
      b8 init(
    };

  }		// -----  end of namespace ecs  ----- 

}		// -----  end of namespace roxi  ----- 

