// =====================================================================================
//
//       Filename:  vk_descriptors.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-05-06 8:11:35 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "rx_allocator.hpp"
#include "vk_buffer.hpp"
#include "vk_descriptors.hpp"
#include "vk_allocator.hpp"
#include "vk_resource.hpp"
#include <vulkan/vulkan_core.h>

namespace roxi {
  namespace vk {

    VkBufferUsageFlags get_descriptor_buffer_usage_flags(const DescriptorBufferType type) {
      static const VkBufferUsageFlags _s_usage_flags[] = 
      { VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
      , VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
      , VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
      , VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
      };
      return _s_usage_flags[(u32)type];
    }

    const VkDescriptorType get_descriptor_type(const DescriptorBufferType type) {
      static const VkDescriptorType _s_descriptor_types[] = 
      { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
      , VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
      , VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
      , VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
      };
      return _s_descriptor_types[(u32)type];
    }

    // sketchy hacky algorithm
    const VkDescriptorType get_descriptor_type(const gpu::BufferType type) {
      static const VkDescriptorType _s_descriptor_types[] = 
      { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
      , VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
      };
      const u32 idx = MIN((u32)type / 2, 2);
      return _s_descriptor_types[idx];
    }

    const VkDescriptorType get_descriptor_type(const gpu::ImageType type) {
      static const VkDescriptorType _s_descriptor_types[] = 
      { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
      , VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
      };
      const u32 idx = MIN((u32)type / 2, 2);
      return _s_descriptor_types[idx];
    }

    b8 DescriptorBufferArena::init(Context* context, const DescriptorBufferType type, const VkDeviceSize descriptor_size, const VkDeviceSize offset_alignment, const u32 count) {
      _descriptor_size = descriptor_size;
      _buffer_size = count * descriptor_size;
      _type = type;
      RX_CHECK
        ( _descriptor_buffer
            .init
              ( context
              , _buffer_size
              , gpu::BufferType::DescriptorBuffer
              , get_descriptor_buffer_usage_flags(type)
              )
        , "failed to init descriptor buffer"
        );
      return true;
    }

    b8 DescriptorBufferArena::bind(mem::Allocation memory) {
      RX_CHECK
        ( _descriptor_buffer.bind(memory)
        , "failed to bind uniform host level descriptor buffer to memory"
        );
      return true;
    }

    DescriptorAllocation DescriptorBufferArena::allocate(Context* context, const u64 descriptor_count, const u64 descriptor_offset_alignment) {
      const auto descriptor_size = (descriptor_count * _descriptor_size) + descriptor_offset_alignment;
#if defined (RX_USE_VK_LOCK_FREE_MEMORY)
      auto offset = _descriptor_counter.add(descriptor_size);
      RX_ASSERT
        ( ((offset + descriptor_size) < _buffer_size)
        , "descriptor buffer overflow!"
        );
#else
      auto offset = _descriptor_counter;
      _descriptor_counter += descriptor_size;
      RX_ASSERT
        ( (_descriptor_counter < _buffer_size)
        , "descriptor buffer overflow!");
#endif
      DescriptorAllocation result;
      result.offset = ALIGN_POW2(offset, descriptor_offset_alignment);
      result.size = descriptor_count * _descriptor_size;

      return result;
    }


    void DescriptorBufferArena::clear() {
#if defined (RX_USE_VK_LOCK_FREE_MEMORY)
      _descriptor_counter.reset();
#else 
      _descriptor_counter = 0;
#endif
    }

    b8 DescriptorBufferArena::terminate() {
      _descriptor_buffer.terminate();
      return true;
    }

    b8 DescriptorSetLayoutCreation::init() {
      _bindings.move_ptr(ALLOCATE(sizeof(BindingInfo) * s_max_descriptor_set_layout_bindings));
      RX_CHECK
        ( _bindings.get_buffer() != nullptr
        , "could not allocate bindings for Descriptor Set Layout Builder"
        );
      return true;
    }

    b8 DescriptorSetLayoutCreation::add_binding(VkDescriptorSetLayoutBinding binding) {
      RX_CHECK
        ( _bindings.get_size() != s_max_descriptor_set_layout_bindings
        , "tried to add too many bindings to DescriptorSetLayoutBuilder"
        );
      *(_bindings.push(1)) = binding;
      return true;
    }

    VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreation::get_create_info() const {
      VkDescriptorSetLayoutCreateInfo info
      {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO
      , nullptr
      , _flags
      , _bindings.get_size()
      , _bindings.get_buffer()
      };
      return info;
    }

    void DescriptorSetLayoutCreation::clear_bindings() {
      _bindings.clear();
    }

    void DescriptorSetLayoutCreation::clear_flags() {
      _flags = 0;
    }

    void DescriptorSetLayoutCreation::clear() {
      _flags = 0;
      _bindings.clear();
    }


    b8 DescriptorSetLayoutCreation::terminate() {
      _bindings.clear();
      FREE(_bindings.get_buffer());
      return true;
    }

    b8 DescriptorSetLayout::init(Context* context, const DescriptorSetLayoutCreation& creation) {
      VkDescriptorSetLayoutCreateInfo create_info = creation.get_create_info();
      VK_CHECK(context->get_device().get_device_function_table()
        .vkCreateDescriptorSetLayout
          ( context->get_device().get_device()
          , &create_info
          , CALLBACKS()
          , &_descriptor_set_layout
          )
        , "failed to create descriptor set layout in DescriptorSetLayout::init(Context*, const DescriptorSetLayoutCreation&)");
      const u32 binding_count = create_info.bindingCount;
      VkDescriptorType* const bindings_begin = _bindings.push(binding_count);
      for(u32 i = 0; i < binding_count; i++) {
        bindings_begin[i] = create_info.pBindings[i].descriptorType;
      }
      return true;

    }

    VkDescriptorSetLayout DescriptorSetLayout::get_descriptor_set_layout() const {
      return _descriptor_set_layout;
    }

    VkDeviceSize DescriptorSetLayout::get_layout_size(Context* context) const {
      VkDeviceSize result{};
      context->get_device().get_device_function_table()
        .vkGetDescriptorSetLayoutSizeEXT
          ( context->get_device().get_device()
          , _descriptor_set_layout
          , &result
          );
      return result;
    }

    const u32 DescriptorSetLayout::get_num_bindings() const {
      return _bindings.get_size();
    }

    VkDescriptorType DescriptorSetLayout::get_layout_binding_type(u32 binding) const {
      RX_ASSERT
        ( binding < get_num_bindings()
        , "tried to index too high a binding in DescriptorSetLayout"
        );

      return _bindings[binding];
    }

    VkDeviceSize DescriptorSetLayout::get_layout_binding_offset(Context* context, u32 binding) const {
      VkDeviceSize result{};
      RX_ASSERT
        ( binding < get_num_bindings()
        , "tried to index too high a binding in DescriptorSetLayout"
        );
      context->get_device().get_device_function_table()
        .vkGetDescriptorSetLayoutBindingOffsetEXT
          ( context->get_device().get_device()
          , _descriptor_set_layout
          , binding
          , &result
          );
      return result;
    }

    DescriptorSetLayout::~DescriptorSetLayout() {
      RX_ASSERT
        ( _descriptor_set_layout == VK_NULL_HANDLE
        , "Descriptor Set Layout not properly destroyed before object is cleared from stack!"
        );
    }

    b8 DescriptorSetLayout::terminate(Context* context) {
      if(_descriptor_set_layout == VK_NULL_HANDLE) {
        return false;
      }
      context->get_device()
        .get_device_function_table()
        .vkDestroyDescriptorSetLayout
          ( context->get_device().get_device()
          , _descriptor_set_layout
          , CALLBACKS());
      _descriptor_set_layout = VK_NULL_HANDLE;
      return true;
    }

    b8 DescriptorPool::init(Context* context, const u32 uniform_buffer_count, const u32 storage_buffer_count, const u32 texture_count, const u32 storage_image_count) {

      _context = context;

      mem::MemoryPoolBuilder memory_builder;
      
      RX_CHECK(memory_builder.init(context)
          , "failed to initialize MemoryPoolBuilder in DescriptorPool::init()");

      const u32 ubo_descriptor_size = memory_builder.get_descriptor_buffer_properties().uniformBufferDescriptorSize;
      const u32 storage_descriptor_size = memory_builder.get_descriptor_buffer_properties().storageBufferDescriptorSize;
      const u32 texture_descriptor_size = memory_builder.get_descriptor_buffer_properties().combinedImageSamplerDescriptorSize;
      const u32 image_descriptor_size = memory_builder.get_descriptor_buffer_properties().storageImageDescriptorSize;
      const u32 descriptor_buffer_offset = memory_builder.get_descriptor_buffer_properties().descriptorBufferOffsetAlignment;

      u32 bucket_ids[4];

      if(uniform_buffer_count != 0) {
        RX_TRACE("creating ubo descriptor arena");
        _descriptor_arenas[UniformArenaIndex].init(context, DescriptorBufferType::Uniform, ubo_descriptor_size, descriptor_buffer_offset, uniform_buffer_count);  
        RX_TRACE("registering ubo memory bucket");
        bucket_ids[UniformArenaIndex] = memory_builder.register_bucket(ubo_descriptor_size * uniform_buffer_count, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT);
      }

      if(storage_buffer_count != 0) {
        RX_TRACE("creating storage descriptor arena");
        _descriptor_arenas[StorageArenaIndex].init(context, DescriptorBufferType::Storage, storage_descriptor_size, descriptor_buffer_offset, storage_buffer_count);  
        RX_TRACE("registering storage memory bucket");
        bucket_ids[StorageArenaIndex] = memory_builder.register_bucket(storage_descriptor_size * storage_buffer_count, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT);
      }

      if(texture_count != 0) {
        RX_TRACE("creating texture descriptor arena");
        _descriptor_arenas[TextureArenaIndex].init(context, DescriptorBufferType::CombinedImageSampler, texture_descriptor_size, descriptor_buffer_offset, texture_count);  
        RX_TRACE("registering texture memory bucket");
        bucket_ids[TextureArenaIndex] = memory_builder.register_bucket(texture_descriptor_size * texture_count, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT);
      }

      if(storage_image_count != 0) {
        RX_TRACE("creating image descriptor arena");
        _descriptor_arenas[ImageArenaIndex].init(context, DescriptorBufferType::StorageImage, image_descriptor_size, descriptor_buffer_offset, storage_image_count);  
        RX_TRACE("registering storage image memory bucket");
        bucket_ids[ImageArenaIndex] = memory_builder.register_bucket(image_descriptor_size * storage_image_count, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT);
      }

      RX_TRACE("building descriptor memory pool");
      RX_CHECK(memory_builder.build(&_memory_pool),
          "failed to build MemoryPool for DescriptorPool");

      if(uniform_buffer_count != 0) {
        RX_TRACE("binding ubo descriptor memory");
        const u32 bucket_id = bucket_ids[UniformArenaIndex];
        RX_CHECK(_descriptor_arenas[UniformArenaIndex].bind(_memory_pool.allocate(bucket_id, _descriptor_arenas[bucket_id].get_buffer_byte_size())), "failed to bind ubo descriptor buffer");
      }

      if(storage_buffer_count != 0) {
        RX_TRACE("binding storage descriptor memory");
        const u32 bucket_id = bucket_ids[StorageArenaIndex];
        RX_CHECK(_descriptor_arenas[StorageArenaIndex].bind(_memory_pool.allocate(bucket_id, _descriptor_arenas[bucket_id].get_buffer_byte_size())), "failed to bind storage descriptor buffer");
      }

      if(texture_count != 0) {
        RX_TRACE("binding texture descriptor memory");
        const u32 bucket_id = bucket_ids[TextureArenaIndex];
        RX_CHECK(_descriptor_arenas[TextureArenaIndex].bind(_memory_pool.allocate(bucket_id, _descriptor_arenas[bucket_id].get_buffer_byte_size())), "failed to bind texture descriptor buffer");
      }

      if(storage_image_count != 0) {
        RX_TRACE("binding image descriptor memory");
        const u32 bucket_id = bucket_ids[ImageArenaIndex];
        RX_CHECK(_descriptor_arenas[ImageArenaIndex].bind(_memory_pool.allocate(bucket_id, _descriptor_arenas[bucket_id].get_buffer_byte_size())), "failed to bind storage image descriptor buffer");
      }

      _descriptor_offset_alignment = descriptor_buffer_offset;

      return true;
    }

    const b8 DescriptorPool::get_descriptor(const vk::Buffer& buffer, const gpu::BufferType type, const vk::DescriptorAllocation allocation, void* dst) {
      VkDescriptorAddressInfoEXT address_info
        { VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT
        , nullptr
        , buffer.get_device_address()
        , buffer.get_size()
        , VK_FORMAT_UNDEFINED
        };
  
      const VkDescriptorType descriptor_type = vk::get_descriptor_type(type);
      VkDescriptorGetInfoEXT get_info{};
      get_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
      get_info.type = descriptor_type;
      switch(descriptor_type) {
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
          get_info.data.pUniformBuffer = &address_info;
          break;
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
          get_info.data.pStorageBuffer = &address_info;
          break;
        default:
          LOG("incorrect descriptor type found in get_descriptor(const BufferHandle, const gpu::BufferType, const vk::DescriptorAllocation)", Error);
        break;
      }
  
      _context->get_device().get_device_function_table()
        .vkGetDescriptorEXT
          ( _context->get_device().get_device()
          , &get_info
          , allocation.size
          , (void*)((u8*)dst + allocation.offset)
          );
  
      return true;
    }
      
  

    const u32 DescriptorPool::get_offset_alignment() const {
      return _descriptor_offset_alignment;
    }

    b8 DescriptorPool::terminate() {
      for(u32 i = 0; i < 4; i++) {
        _descriptor_arenas[i].terminate();
      }
      _memory_pool.terminate(_context);
      return true;
    }


//    b8 DescriptorPoolBuilder::init(Context* context) {
//      _context = context;
//      _memory_builder.init(_context);
//      _ubo_descriptor_size = _memory_builder.get_descriptor_buffer_properties().uniformBufferDescriptorSize;
//      _storage_descriptor_size = _memory_builder.get_descriptor_buffer_properties().storageBufferDescriptorSize;
//      _texture_descriptor_size = _memory_builder.get_descriptor_buffer_properties().combinedImageSamplerDescriptorSize;
//      _image_descriptor_size = _memory_builder.get_descriptor_buffer_properties().storageImageDescriptorSize;
//      _descriptor_buffer_offset = _memory_builder.get_descriptor_buffer_properties().descriptorBufferOffsetAlignment;
//      return true;
//    }
//
//    DescriptorPool::DescriptorBufferHandle DescriptorPoolBuilder::add_descriptor_buffer(const DescriptorBufferType type, const DescriptorBufferLevel level, const u32 descriptor_count) {
//      const u32 result = _init_infos.get_size();
//      descriptor_buffer_init_info& info = *(_init_infos.push(1));
//      info.type = type;
//      info.level = level;
//      info.count = descriptor_count;
//      return result;
//    }
//
//    b8 DescriptorPoolBuilder::build(DescriptorPool* const pool) {
//      if(pool->_descriptor_arenas.get_buffer() != nullptr) {
//        return false;
//      }
//      const u32 descriptor_buffer_count = _init_infos.get_size();
//      StackArray<u32> bucket_ids{};
//      u32* const bucket_id_begin = bucket_ids.push(descriptor_buffer_count);
//
//      pool->_descriptor_types.move_ptr(ALLOCATE(sizeof(DescriptorBufferType) * descriptor_buffer_count));
//      DescriptorBufferType* types_begin = pool->_descriptor_types.push(descriptor_buffer_count);
//
//      pool->_descriptor_levels.move_ptr(ALLOCATE(sizeof(DescriptorBufferLevel) * descriptor_buffer_count));
//      DescriptorBufferLevel* levels_begin = pool->_descriptor_levels.push(descriptor_buffer_count);
//
//      pool->_descriptor_arenas.move_ptr(ALLOCATE(sizeof(DescriptorBufferArena) * descriptor_buffer_count));
//      DescriptorBufferArena* const descriptor_arenas_begin = pool->_descriptor_arenas.push(descriptor_buffer_count);
//
//      for(u32 i = 0; i < descriptor_buffer_count; i++) {
//        const DescriptorBufferType type = _init_infos[i].type;
//        const u32 descriptor_size 
//          = type == DescriptorBufferType::Uniform ?
//            _ubo_descriptor_size 
//            : type == DescriptorBufferType::Storage ?
//              _storage_descriptor_size 
//              : type == DescriptorBufferType::CombinedImageSampler ?
//                _texture_descriptor_size
//                : type == DescriptorBufferType::StorageImage ?
//                  _image_descriptor_size
//                  : MAX_u32;
//        const DescriptorBufferLevel level = _init_infos[i].level;
//        const u32 count = _init_infos[i].count;
//      
//        types_begin[i] = type;
//        levels_begin[i] = level;
//
//        descriptor_arenas_begin[i].init
//          ( _context
//          , type
//          , level
//          , descriptor_size
//          , _descriptor_buffer_offset
//          , count
//          );
//        VkMemoryPropertyFlags allocate_flags 
//          = level == DescriptorBufferLevel::Host ?
//            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
//            : level == DescriptorBufferLevel::Device ?
//              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
//              : 0;
//
//        bucket_id_begin[i] = _memory_builder.register_bucket(descriptor_size * count, allocate_flags);
//      }
//      _memory_builder.build(&pool->_memory_pool);
//      for(u32 i = 0; i < descriptor_buffer_count; i++) {
//        descriptor_arenas_begin[i].bind(pool->_memory_pool.allocate(bucket_id_begin[i], descriptor_arenas_begin[i].get_buffer_byte_size()));
//      }
//      pool->_descriptor_offset_alignment = _descriptor_buffer_offset;
//      return true;
//    }
//
//    b8 DescriptorPoolBuilder::terminate() {
//      _memory_builder.terminate();
//      _context = nullptr;
//      return true;
//    }

//        OLD CODE for getting descriptors from combined image sampler
//
//        VkDescriptorGetInfoEXT get_info{};
//        get_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
//
//        void* _descriptor_buffer_ptr = _descriptor_buffer.map();
//        for(u64 i = 0; i < descriptor_count; i++) {
//          VkDescriptorImageInfo image_info = descriptor[i].get_image_info();
//          image_info.sampler = create_default_sampler(context);
//          get_info.data.pCombinedImageSampler = &image_info;
//          context->get_device().get_device_function_table()
//            .vkGetDescriptorEXT
//              ( context->get_device().get_device()
//              , &get_info
//              , _descriptor_size
//              , (void*)((u8*)_descriptor_buffer_ptr + result.offset + (i * _descriptor_size))
//              );
//        }
//        _descriptor_buffer.unmap();
//
//        return result;


      //DescriptorAllocation write_descriptors(Context* context, const Buffer* descriptor, const u64 descriptor_count, const u64 descriptor_offset_alignment) {
      //  DescriptorAllocation result;
      //  result.offset = allocate_aligned_descriptors(descriptor_count);
      //  result.size = descriptor_count * _descriptor_size;
      //  VkDescriptorAddressInfoEXT address_info{};
      //  address_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT;
      //  address_info.format = VK_FORMAT_UNDEFINED;

      //  VkDescriptorGetInfoEXT get_info{};
      //  get_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
      //  get_info.data.pUniformBuffer = &address_info;

      //  void* _descriptor_buffer_ptr = _descriptor_buffer.map();
      //  for(u64 i = 0; i < descriptor_count; i++) {
      //    address_info.address = descriptor[i].get_device_address();
      //    address_info.range = descriptor[i].get_size();

      //    context->get_device().get_device_function_table()
      //      .vkGetDescriptorEXT
      //        ( context->get_device().get_device()
      //        , &get_info
      //        , _descriptor_size
      //        , (void*)((u8*)_descriptor_buffer_ptr + result.offset + (i * _descriptor_size))
      //        );
      //  }
      //  _descriptor_buffer.unmap();

      //  return result;
      //} 


  }		// -----  end of namespace vk  ----- 
}		// -----  end of namespace roxi  ----- 
