// =====================================================================================
//
//       Filename:  rx_graphics.hpp
//
//    Description:  graphics system
//
//        Version:  1.0
//        Created:  2024-07-24 9:21:43 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "rx_gpu_device.hpp"
#include "rx_thread_pool.hpp"
#include "rx_vocab.h"
#include "rx_renderer.hpp"
#include "rx_system.hpp"


namespace roxi {
  namespace systems {
    namespace graphics {
      DEFINE_JOB(init);

      DEFINE_JOB(update);

      DEFINE_JOB(terminate);
    }
  }


  class Graphics : public ISystem {
  private:
    friend b8 systems::graphics::init(void*,u64,u64);
    friend b8 systems::graphics::update(void*,u64,u64);
    friend b8 systems::graphics::terminate(void*,u64,u64);
    TestRenderer _renderer;
  public:
    Job get_init_job(Counter* counter, const u64 start, const u64 end) override {
      RX_TRACE("getting Graphics initialization job");
      Job result;
      result
        .set_job_counter(counter)
        .set_entry_point(systems::graphics::init)
        .set_job_start(start)
        .set_job_end(end)
        .set_job_success(job_success_increment)
        .set_job_failure(job_failed_no_op)
        .set_obj((void*)this);
      return result;
    }

    Job get_update_job(Counter* counter, const u64 start, const u64 end) override {
      RX_TRACE("getting Graphics update job");
      Job result;
      result
        .set_job_counter(counter)
        .set_entry_point(systems::graphics::update)
        .set_job_start(start)
        .set_job_end(end)
        .set_job_success(job_success_increment)
        .set_job_failure(job_failed_no_op)
        .set_obj((void*)this);
      return result;
    }

    Job get_terminate_job(Counter* counter, const u64 start, const u64 end) override {
      RX_TRACE("getting Graphics terminate job");
      Job result;
      result
        .set_job_counter(counter)
        .set_entry_point(systems::graphics::terminate)
        .set_job_start(start)
        .set_job_end(end)
        .set_job_success(job_success_increment)
        .set_job_failure(job_failed_no_op)
        .set_obj((void*)this);
      return result;
    }
  };
  namespace systems {
    namespace graphics {

//      static TestRenderer _s_renderer;
//      static constexpr u32 InitialWindowWidth = 720;
//      static constexpr u32 InitialWindowHeight = 480;

      // param = GPUDevice*, start = GPUDevice::QueueHandle

      DEFINE_JOB(init) {
        RX_TRACE("initializing graphics");

        Graphics& graphics = *(Graphics*)param;
        GPUDevice* gpu_device = (GPUDevice*)(INT2PTR(end));

        RX_CHECK(graphics._renderer.init(gpu_device, start)
            , "TestRenderer::init() failed in graphics system");
        return true;
      }

      DEFINE_JOB(update) {
        Graphics& graphics = *(Graphics*)param;
        RX_TRACE("updating graphics");
        RX_CHECKF(graphics._renderer.update(start)
            , "TestRenderer::update() at index = %u failed"
            , start);
        return true;
      }

      DEFINE_JOB(terminate) {
        RX_TRACE("terminating graphics");
        return true;
      }

    }		// -----  end of namespace graphics  ----- 
  }		// -----  end of namespace systems  ----- 
//  RX_DECLARE_SYSTEM(graphics);

}		// -----  end of namespace roxi  ----- 
