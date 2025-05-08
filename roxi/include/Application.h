// =====================================================================================
//
//       Filename:  Application.h
//
//    Description:  
//
//        Version:  1.0
//        Created:  2023-12-03 6:16:02 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once

#include "rx_frame_manager.hpp"
#include "rx_graphics.hpp"
#include "window.h"
#include <rpcndr.h>

//#define RX_USE_LOCK_FREE_MEMORY
//#define RX_USE_VK_LOCK_FREE_MEMORY
//#define RX_USE_LOGGING
//#define RX_USE_ALLOCATION_CALLBACKS
#define WINDOW_WIDTH 720
#define WINDOW_HEIGHT 480

// frame time in ns
#define RX_FRAME_TIME 1000

namespace roxi {

  class Application {
  private:
    using system_list_t = StackArray<ISystem*>;
  public:
    using system_handle_t = typename system_list_t::index_t;

    b8 run() noexcept;

    FrameManager& obtain_frame_manager();
    const frame::ID get_current_front_frame() const;


    // make ecs queries here too

  private:
    b8 init_sequence();
    b8 update_sequence();
    b8 terminate_sequence();

    template<class SystemT>
    const system_handle_t create_system() {
      ISystem* system_ptr = (ISystem*)_arena.push(sizeof(SystemT));
      RX_RETURN(system_ptr != nullptr
          , "failed to allocate a system"
          , lofi::index_type_max<system_handle_t>::value
          );
      new(system_ptr) SystemT();
      const system_handle_t result = _systems.get_size();
      *(_systems.push(1)) = system_ptr;
      RX_TRACEF("current system count = %u", _systems.get_size());
      return result;
    }

    void handle_input(const u64 frame_id) noexcept;

    GPUDevice _gpu_device;
    FrameManager _frame_manager;
    system_list_t _systems;
    frame::ID _current_front_frame{0};
    Arena _arena;
#if OS_WINDOWS
    win::Window _window;
#endif
  };

}		// -----  end of namespace roxi  ----- 
