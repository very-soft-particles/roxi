// =====================================================================================
//
//       Filename:  rx_gpu_loader.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2025-01-19 7:05:22 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Robyn Gray (rg), ...
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "rx_gpu_device.hpp"
#include "vk_command.hpp"
#include "vk_device.h"
#include "vk_resource.hpp"
#include "vk_sync.hpp"


namespace roxi {
 
  class GPULoader {
  private:
    GPUDevice*                                                                _device;
    Counter                                                                   _top[2];
    GPUDevice::BufferHandle                                 _staging_buffer_handle[2];
    GPUDevice::CommandArenaHandle<vk::CommandType::MainTransfer>   _command_arenas[2];
    GPUDevice::CommandBufferHandle<vk::CommandType::MainTransfer> _command_buffers[2];
    GPUDevice::SemaphoreHandle                                 _pending_semaphores[2];
    u32                                                     _current_write_buffer = 0;
    u8*                                                          _mapped_buffer = nullptr;


  public:

    b8 init(GPUDevice* device, const u32 size) {
      _device = device;
      _staging_buffer_handle[0] = _device->create_buffer(gpu::BufferType::StagingBuffer, size);
      _staging_buffer_handle[1] = _device->create_buffer(gpu::BufferType::StagingBuffer, size);

      _command_arenas[0] = _device->create_command_arena<vk::CommandType::MainTransfer>(1);
      _command_arenas[1] = _device->create_command_arena<vk::CommandType::MainTransfer>(1);

      _command_buffers[0] = _device->obtain_command_arena<vk::CommandType::MainTransfer>(_command_arenas[0]).allocate_command_buffers(vk::CommandBufferLevelType::Primary, 1);
      _command_buffers[1] = _device->obtain_command_arena<vk::CommandType::MainTransfer>(_command_arenas[1]).allocate_command_buffers(vk::CommandBufferLevelType::Primary, 1);

      _semaphore_handle[0] = _device->create_semaphore();
      _semaphore_handle[1] = _device->create_semaphore();

      return true;
    }

    b8 write_data(const void* src, const u64 size) {
      const u64 current_top = _top[_current_write_buffer].add(size);
      if(current_top + size >= _buffer_size) {
        LOGF(Error, "staging buffer was not large enough to write data, current size = %llu, size after load = %llu", current_top, current_top + size);
        return false;
      }

      u8* dst_ptr = _mapped_buffer + current_top;
      MEM_COPY(dst_ptr, src, size);

      return true;
    }

    const b8 upload_data_and_swap() {
      const u32 current_upload_buffer = _current_write_buffer;
      _current_write_buffer ^= 1;

      _device->obtain_command_buffer<vk::CommandType::MainTransfer>(const CommandHandle<CommandT> handle, const CommandBufferHandle<CommandT> buffer_handle)

      _device->submit<vk::CommandType::MainTransfer>(vk::QueueIndices::MainTransferIndex, &_command_buffers[current_upload_buffer], 1);
      
    }





    b8 terminate() {

      return true;
    }


  };

}		// -----  end of namespace roxi  ----- 
