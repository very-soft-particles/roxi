// =====================================================================================
//
//       Filename:  vk_sync.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-07-02 6:26:36 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once

#include "rx_vocab.h"
#include "vk_consts.h"
#include "vk_context.h"


namespace roxi {
  
  namespace vk {

    class Semaphore {
    private:
      VkSemaphore _semaphore = VK_NULL_HANDLE;
    public:
      b8 init(Context* context) {
        VkSemaphoreCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;

        VK_CHECK(context->get_device().get_device_function_table()
          .vkCreateSemaphore(context->get_device().get_device(), &create_info, CALLBACKS(), &_semaphore)
          , "failed to create VkSemaphore");

        return true;
      }

      const VkSemaphore& get_semaphore() const {
        return _semaphore;
      }

      b8 terminate(Context* context) {
        context->get_device().get_device_function_table()
          .vkDestroySemaphore(context->get_device().get_device()
              , _semaphore, CALLBACKS());
        return true;
      }
    };

    class TimelineSemaphore {
    private:
      VkSemaphore _semaphore = VK_NULL_HANDLE;
    public:
      b8 init(Context* context, u32 initial_value) {
        VkSemaphoreTypeCreateInfo type_create_info{};
        type_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
        type_create_info.pNext = nullptr;
        type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        type_create_info.initialValue = initial_value;

        VkSemaphoreCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        create_info.pNext = &type_create_info;
        create_info.flags = 0;

        VK_CHECK(context->get_device().get_device_function_table()
          .vkCreateSemaphore(context->get_device().get_device(), &create_info, CALLBACKS(), &_semaphore)
          , "failed to create vulkan timeline semaphore");

        return true;
      }

      const VkSemaphore& get_semaphore() const {
        return _semaphore;
      }

      const u64 get_count(Context* context) const {
        u64 value = MAX_u64;
        VK_ASSERT(context->get_device().get_device_function_table()
          .vkGetSemaphoreCounterValue(context->get_device().get_device()
              , _semaphore, &value)
          , "failed to get semaphore count");
        return value;
      }

      b8 terminate(Context* context) {
        context->get_device().get_device_function_table()
          .vkDestroySemaphore(context->get_device().get_device()
              , _semaphore, CALLBACKS());
        return true;
      }
    };

    class Fence {
    private:
      VkFence _fence = VK_NULL_HANDLE;
    public:
      b8 init(Context* context, b8 signaled = false) {
        VkFenceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
        RX_TRACE("vkCreateFence...");
        VK_CHECK(context->get_device().get_device_function_table()
          .vkCreateFence(context->get_device().get_device()
              , &create_info, CALLBACKS(), &_fence)
          , "failed to create VkFence");
        return true;
      }

      const b8 is_signaled(Context* context) const {
        if(context->get_device().get_device_function_table()
          .vkGetFenceStatus
            ( context->get_device().get_device()
            , _fence
            ) == VK_SUCCESS
          ) 
        {
          return true;
        } else
        return false;
      }

      void reset(Context* context) {
        context->get_device().get_device_function_table()
          .vkResetFences(context->get_device().get_device(), 1, &_fence);
      }

      const VkFence& get_fence() const {
        return _fence;
      }

      b8 terminate(Context* context) {
        context->get_device().get_device_function_table()
          .vkDestroyFence(context->get_device().get_device()
              , _fence, CALLBACKS());
        return true;
      }
    };


    DEFINE_JOB_FAILURE(wait_failure) {
      // Logging?
    }


    DEFINE_JOB_SUCCESS(wait_success) {
      // Logging?
      Counter* _counter = ((Counter*)counter);
      _counter++;
    }

    DEFINE_JOB(wait_job_func) {
      Context& context = *((Context*)param);
      Fence& fence = *((Fence*)INT2PTR(start));
      Counter* counter = ((Counter*)INT2PTR(end));
      if(fence.is_signaled(&context)) {
        return true;
      }
      Job job{};
      job
        .set_entry_point(wait_job_func)
        .set_obj(param)
        .set_job_start(start)
        .set_job_end(end)
        .set_job_failure(wait_failure)
        .set_job_counter(counter);
      RX_FIBER_KICK_LOW_PRIORITY_JOB(job);
      return false;
    }

    static void wait_on_fence(Context* context, Fence* fence, Counter* counter) {
      RX_TRACE("waiting on vulkan fence");
#if defined(RX_USE_VK_LOCK_FREE_MEMORY)
      Job job{};
      job
        .set_entry_point(wait_job_func)
        .set_obj((void*)context)
        .set_job_counter(counter)
        .set_job_failure(wait_failure)
        .set_job_start(PTR2INT((void*)fence))
        .set_job_end(PTR2INT((void*)counter));
      RX_FIBER_KICK_LOW_PRIORITY_JOB(job);
      RX_FIBER_WAIT(counter, 1);
#else
      context->get_device().get_device_function_table().vkWaitForFences(context->get_device().get_device()
          , 1, &(fence->get_fence()), VK_TRUE, 1000000);
#endif
      RX_TRACE("returning from waiting fiber");
    }

  }		// -----  end of namespace vk  ----- 
}		// -----  end of namespace roxi  ----- 
