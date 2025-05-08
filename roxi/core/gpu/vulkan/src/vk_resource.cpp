// =====================================================================================
//
//       Filename:  vk_resource.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2025-02-17 1:30:14 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Robyn Gray (rg), ...
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "vk_resource.hpp"
#include "rx_allocator.hpp"
#include "vk_allocator.hpp"
#include "vk_buffer.hpp"
#include "vk_image.hpp"


namespace roxi {
 
  namespace vk {

    b8 ResourcePool::init(Context* context, const u32 buffer_info_count, const gpu::ResourceInfo* buffer_infos, const u32 image_info_count, const gpu::ResourceInfo* image_infos) {
      _context = context;

      u8* pool_memory = (u8*)ALLOCATE(sizeof(Buffer) * (buffer_info_count + 1) + sizeof(Image) * (image_info_count + 1) + 128 * sizeof(VkSampler));
      mem::MemoryPoolBuilder memory_builder;
      RX_CHECK(memory_builder.init(context)
          , "failed to init memory builder in ResourcePool::init()");

      _buffer_pool.clear();
      _buffer_pool.move_ptr(pool_memory);
      pool_memory += sizeof(Buffer) * (buffer_info_count + 1);


      _image_pool.clear();
      _image_pool.move_ptr(pool_memory);
      pool_memory += sizeof(Image) * (image_info_count + 1);

      _samplers.clear();
      _samplers.move_ptr(pool_memory);

      StackArray<VkMemoryRequirements2> buffer_memory_reqs;

      Buffer* buffer_begin = _buffer_pool.push(buffer_info_count);
      for(u32 i = 0; i < buffer_info_count; i++) {
        const gpu::ResourceType type = buffer_infos[i].type;

        const gpu::BufferType buffer_type = convert_to_buffer_type(type);

        const VkBufferUsageFlags flags = get_buffer_usage_flags(buffer_type);
        if(!buffer_begin[i].init(_context, buffer_infos[i].buffer.size, buffer_type)) {
          RX_ERROR("failed to init buffer in ResourcePool::init");
          return false;
        }

        VkBufferMemoryRequirementsInfo2 buffer_mem_reqs_info{VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2};
        buffer_mem_reqs_info.buffer = buffer_begin[i].get_buffer();

        buffer_memory_reqs[i].sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;

        _context->get_device().get_device_function_table()
          .vkGetBufferMemoryRequirements2(_context->get_device().get_device()
              , &buffer_mem_reqs_info
              , &buffer_memory_reqs[i]);

        _buffer_arena_ids[(u32)buffer_type] = memory_builder.register_bucket(buffer_memory_reqs[i].memoryRequirements, get_memory_type_flags(convert_to_buffer_type(buffer_infos[i].type)));
      }

      StackArray<VkMemoryRequirements2> image_memory_reqs;
      Image* image_begin = _image_pool.push(image_info_count);
      for(u32 i = 0; i < image_info_count; i++) {
        const gpu::ResourceType type = image_infos[i].type;
        const gpu::ImageType image_type = convert_to_image_type(type);

        const VkExtent3D image_size = VkExtent3D{image_infos[i].image.width, image_infos[i].image.height, image_infos[i].image.depth};

        if(!image_begin[i].init(_context, image_size, image_type, 1, VK_SAMPLE_COUNT_1_BIT, false)) {
          RX_ERROR("failed to init image in ResourcePool::create_image");
          return false;
        }


        VkImageMemoryRequirementsInfo2 image_mem_reqs_info{VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2};
        image_mem_reqs_info.image = image_begin[i].get_image();

        image_memory_reqs[i].sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
        _context->get_device().get_device_function_table()
          .vkGetImageMemoryRequirements2(_context->get_device().get_device()
              , &image_mem_reqs_info
              , &image_memory_reqs[i]);

        _image_arena_ids[(u8)convert_to_image_type(image_infos[i].type)] = memory_builder.register_bucket(image_memory_reqs[i].memoryRequirements, get_memory_type_flags(image_type));
      }

      RX_CHECK(memory_builder.build(&_memory_pool)
          , "failed to build memory pool in ResourcePool::init()");

      RX_CHECK(memory_builder.terminate()
          , "failed to terminate memory builder in ResourcePool::init()");

      for(u32 i = 0; i < buffer_info_count; i++) {
        Buffer& buffer = obtain_buffer(i);
        
        mem::Allocation allocation = _memory_pool.allocate(get_buffer_arena_id(buffer.get_buffer_type()), buffer_memory_reqs[i].memoryRequirements.size, buffer_memory_reqs[i].memoryRequirements.alignment);
        RX_CHECK(buffer.bind(allocation)
            , "failed to bind vk buffer");
      }

      for(u32 i = 0; i < image_info_count; i++) {
        Image& image = obtain_image(i);
        mem::Allocation allocation = _memory_pool.allocate(get_image_arena_id(image.get_image_type()), image_memory_reqs[i].memoryRequirements.size, image_memory_reqs[i].memoryRequirements.alignment);
        image.bind(allocation);
      }
      return true;
    }

    const ResourcePool::SamplerHandle ResourcePool::create_sampler(const float max_anisotropy, const VkFilter min_filter, const VkFilter mag_filter, const float min_lod, const float max_lod, const float mip_lod_bias, const VkSamplerMipmapMode mip_map_mode, const VkSamplerAddressMode address_mode_u, const VkSamplerAddressMode address_mode_v, const VkSamplerAddressMode address_mode_w, const VkCompareOp compare_op) {

      const SamplerHandle result = _samplers.get_size();

      VkSamplerCreateInfo create_info{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
      create_info.addressModeU = address_mode_u;
      create_info.addressModeV = address_mode_v;
      create_info.addressModeW = address_mode_w;
      create_info.mipmapMode = mip_map_mode;
      create_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
      create_info.compareEnable = compare_op == VK_COMPARE_OP_NEVER ? VK_FALSE : VK_TRUE;
      create_info.compareOp = compare_op;
      create_info.mipLodBias = mip_lod_bias;
      create_info.minLod = min_lod;
      create_info.maxLod = max_lod;
      create_info.minFilter = min_filter;
      create_info.magFilter = mag_filter;
      create_info.maxAnisotropy = max_anisotropy;
      create_info.anisotropyEnable = (u32)max_anisotropy == 0 ? VK_FALSE : VK_TRUE;

      VK_ASSERT(_context->get_device().get_device_function_table()
        .vkCreateSampler(_context->get_device().get_device()
            , &create_info
            , CALLBACKS()
            , _samplers.push(1)
            )
        , "failed to create sampler in ResourcePool");

      return result;
    }

    Buffer& ResourcePool::obtain_buffer(const BufferHandle handle) {
      return _buffer_pool[handle];
    }

    Image& ResourcePool::obtain_image(const ImageHandle handle) {
      return _image_pool[handle];
    }

    void ResourcePool::clear_buffer_pool() {
      const u32 buffer_count = _buffer_pool.get_size();
      for(u32 i = 0; i < buffer_count; i++) {
        _buffer_pool[i].terminate();
      }
      _buffer_pool.clear();
    }

    void ResourcePool::clear_image_pool() {
      const u32 count = _image_pool.get_size();
      for(u32 j = 0; j < count; j++) {
        _image_pool[j].terminate();
      }
      _image_pool.clear();
    }

    void ResourcePool::clear_pools() {
      clear_buffer_pool();
      clear_image_pool();
    }

    void ResourcePool::clear() {
      clear_arenas();
      clear_pools();
    }

    b8 ResourcePool::terminate() {
      clear();
      FREE(_buffer_pool.get_buffer());
      return true;
    }

  }		// -----  end of namespace vk  ----- 

}		// -----  end of namespace roxi  ----- 
