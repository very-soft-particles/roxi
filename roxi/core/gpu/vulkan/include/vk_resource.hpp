// =====================================================================================
//
//       Filename:  vk_resource.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-08-08 2:16:17 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "../../../resource/rx_resource_manager.hpp"
#include "vk_allocator.hpp"
#include "vk_buffer.hpp"
#include "vk_image.hpp"
#include <vulkan/vulkan_core.h>

#define VK_RESOURCE_TYPES(X) VK_BUFFER_TYPES(X) VK_IMAGE_TYPES(X)

namespace roxi {
  namespace gpu {

    static constexpr u64 s_max_resources = KB(4);

#define ENUM(Type) Type, 
    enum class ResourceType : u8 {
        VK_RESOURCE_TYPES(ENUM)
    };

#undef ENUM

    struct ResourceInfo {
      union {
        struct {
          u64                             size;
        } buffer;

        struct {
          u32                             width;
          u32                            height;
          u32                             depth;
        } image;
      };
      ResourceType type;

    };

    static const char* get_vk_resource_type_name(ResourceType type) {
#define STR(Type) if (type == ResourceType::Type) return (const char*)STRINGIFY(Type);
      VK_RESOURCE_TYPES(STR)
#undef STR
        else
          return "";
    }
  }		// -----  end of namespace gpu  -----
  static VkMemoryPropertyFlags get_memory_type_flags(const gpu::BufferType type) {
    static const VkMemoryPropertyFlags _s_flags[] = 
      { VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
      , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
      , VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
      , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
      , VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
      , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
      , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
      };
    return _s_flags[(u8)type];
  }

  static VkMemoryAllocateFlags get_aux_memory_flags(const gpu::BufferType type) {
    static const VkMemoryPropertyFlags _s_flags[] = 
      { VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT
      , VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT
      , VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT
      , VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT
      , 0
      , 0
      , 0
      };
    return _s_flags[(u8)type];
  }


  static VkMemoryPropertyFlags get_memory_type_flags(const gpu::ImageType type) {
    static const VkMemoryPropertyFlags _s_flags[] = 
      { VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
      , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
      , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
      , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
      , 0
      , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
      };
    return _s_flags[(u8)type];
  }

//  static const u32 get_image_format_size(const VkFormat format) {
//    switch (format) {
//      case VK_FORMAT_R4G4_UNORM_PACK8:
//      case VK_FORMAT_R8_UNORM:
//      case VK_FORMAT_R8_SNORM:
//      case VK_FORMAT_R8_USCALED:
//      case VK_FORMAT_R8_SSCALED:
//      case VK_FORMAT_R8_UINT:
//      case VK_FORMAT_R8_SINT:
//      case VK_FORMAT_R8_SRGB:
//        return 1;
//
//      case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
//      case VK_FORMAT_R10X6_UNORM_PACK16:
//      case VK_FORMAT_R12X4_UNORM_PACK16:
//      case VK_FORMAT_A4R4G4B4_UNORM_PACK16:
//      case VK_FORMAT_A4B4G4R4_UNORM_PACK16:
//      case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
//      case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
//      case VK_FORMAT_R5G6B5_UNORM_PACK16:
//      case VK_FORMAT_B5G6R5_UNORM_PACK16:
//      case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
//      case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
//      case VK_FORMAT_R8G8_UNORM:
//      case VK_FORMAT_R8G8_SNORM:
//      case VK_FORMAT_R8G8_USCALED:
//      case VK_FORMAT_R8G8_SSCALED:
//      case VK_FORMAT_R8G8_UINT:
//      case VK_FORMAT_R8G8_SINT:
//      case VK_FORMAT_R8G8_SRGB:
//      case VK_FORMAT_R16_UNORM:
//      case VK_FORMAT_R16_SNORM:
//      case VK_FORMAT_R16_USCALED:
//      case VK_FORMAT_R16_SSCALED:
//      case VK_FORMAT_R16_UINT:
//      case VK_FORMAT_R16_SINT:
//      case VK_FORMAT_R16_SFLOAT:
//        return 2;
//
//      case VK_FORMAT_R10X6G10X6_UNORM_2PACK16:
//      case VK_FORMAT_R12X4G12X4_UNORM_2PACK16:
//      case VK_FORMAT_R16G16_S10_5_NV:
//      case VK_FORMAT_R8G8B8A8_UNORM:
//      case VK_FORMAT_R8G8B8A8_SNORM:
//      case VK_FORMAT_R8G8B8A8_USCALED:
//      case VK_FORMAT_R8G8B8A8_SSCALED:
//      case VK_FORMAT_R8G8B8A8_UINT:
//      case VK_FORMAT_R8G8B8A8_SINT:
//      case VK_FORMAT_R8G8B8A8_SRGB:
//      case VK_FORMAT_B8G8R8A8_UNORM:
//      case VK_FORMAT_B8G8R8A8_SNORM:
//      case VK_FORMAT_B8G8R8A8_USCALED:
//      case VK_FORMAT_B8G8R8A8_SSCALED:
//      case VK_FORMAT_B8G8R8A8_UINT:
//      case VK_FORMAT_B8G8R8A8_SINT:
//      case VK_FORMAT_B8G8R8A8_SRGB:
//      case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
//      case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
//      case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
//      case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
//      case VK_FORMAT_A8B8G8R8_UINT_PACK32:
//      case VK_FORMAT_A8B8G8R8_SINT_PACK32:
//      case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
//      case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
//      case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
//      case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
//      case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
//      case VK_FORMAT_A2R10G10B10_UINT_PACK32:
//      case VK_FORMAT_A2R10G10B10_SINT_PACK32:
//      case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
//      case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
//      case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
//      case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
//      case VK_FORMAT_A2B10G10R10_UINT_PACK32:
//      case VK_FORMAT_A2B10G10R10_SINT_PACK32:
//      case VK_FORMAT_R16G16_UNORM:
//      case VK_FORMAT_R16G16_SNORM:
//      case VK_FORMAT_R16G16_USCALED:
//      case VK_FORMAT_R16G16_SSCALED:
//      case VK_FORMAT_R16G16_UINT:
//      case VK_FORMAT_R16G16_SINT:
//      case VK_FORMAT_R16G16_SFLOAT:
//      case VK_FORMAT_R32_UINT:
//      case VK_FORMAT_R32_SINT:
//      case VK_FORMAT_R32_SFLOAT:
//      case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
//      case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
//        return 4;
//      case VK_FORMAT_R16G16B16_UNORM:
//      case VK_FORMAT_R16G16B16_SNORM:
//      case VK_FORMAT_R16G16B16_USCALED:
//      case VK_FORMAT_R16G16B16_SSCALED:
//      case VK_FORMAT_R16G16B16_UINT:
//      case VK_FORMAT_R16G16B16_SINT:
//      case VK_FORMAT_R16G16B16_SFLOAT:
//        return 6;
//
//      case VK_FORMAT_R16G16B16A16_UNORM:
//      case VK_FORMAT_R16G16B16A16_SNORM:
//      case VK_FORMAT_R16G16B16A16_USCALED:
//      case VK_FORMAT_R16G16B16A16_SSCALED:
//      case VK_FORMAT_R16G16B16A16_UINT:
//      case VK_FORMAT_R16G16B16A16_SINT:
//      case VK_FORMAT_R16G16B16A16_SFLOAT:
//      case VK_FORMAT_R32G32_UINT:
//      case VK_FORMAT_R32G32_SINT:
//      case VK_FORMAT_R32G32_SFLOAT:
//      case VK_FORMAT_R64_UINT:
//      case VK_FORMAT_R64_SINT:
//      case VK_FORMAT_R64_SFLOAT:
//        return 8;
//
//      case VK_FORMAT_R32G32B32_UINT:
//      case VK_FORMAT_R32G32B32_SINT:
//      case VK_FORMAT_R32G32B32_SFLOAT:
//        return 12;
//
//      case VK_FORMAT_R32G32B32A32_UINT:
//      case VK_FORMAT_R32G32B32A32_SINT:
//      case VK_FORMAT_R32G32B32A32_SFLOAT:
//      case VK_FORMAT_R64G64_UINT:
//      case VK_FORMAT_R64G64_SINT:
//      case VK_FORMAT_R64G64_SFLOAT:
//        return 16;
//
//      case VK_FORMAT_R64G64B64_UINT:
//      case VK_FORMAT_R64G64B64_SINT:
//      case VK_FORMAT_R64G64B64_SFLOAT:
//        return 24;
//
//      case VK_FORMAT_R64G64B64A64_UINT:
//      case VK_FORMAT_R64G64B64A64_SINT:
//      case VK_FORMAT_R64G64B64A64_SFLOAT:
//        return 32;
//
//      default:
//        RX_ERROR("undefined vulkan image format when getting format size");
//        return 0;
//    };
//  }

  template<gpu::ResourceType Type>
  static constexpr b8 resource_is_image_type() {
    return (u8)Type >= (u8)gpu::ResourceType::Texture;
  }

  template<gpu::ResourceType Type>
  static constexpr b8 resource_is_buffer_type() {
    return !resource_is_image_type<Type>();
  }


  static b8 resource_is_image_type(const gpu::ResourceType type) {
    return (u8)type >= (u8)gpu::ResourceType::Texture ? true : false;
  }

  static b8 resource_is_buffer_type(const gpu::ResourceType type) {
    return !resource_is_image_type(type);
  }

  // check to ensure that type is buffer or else will crash
  static gpu::BufferType convert_to_buffer_type(const gpu::ResourceType type) {
    static const gpu::BufferType _s_buffer_types[] = 
    { gpu::BufferType::HostUniformBuffer
    , gpu::BufferType::DeviceUniformBuffer
    , gpu::BufferType::HostStorageBuffer
    , gpu::BufferType::DeviceStorageBuffer
    , gpu::BufferType::StagingBuffer
    , gpu::BufferType::IndirectCommand
    , gpu::BufferType::TransferSource
    };
    return _s_buffer_types[(u8)type];
  }

  // check to ensure that type is buffer or else will crash
  static gpu::ImageType convert_to_image_type(const gpu::ResourceType type) {
    static const gpu::ImageType _s_image_types[] = 
    { gpu::ImageType::Texture
    , gpu::ImageType::InputAttachment
    , gpu::ImageType::RenderTarget
    , gpu::ImageType::DepthTarget
    , gpu::ImageType::StorageImage
    , gpu::ImageType::Reference
    };
    return _s_image_types[(u8)type - (u8)gpu::ResourceType::Texture];
  }

  static constexpr u32 RoxiBindlessDescriptorCount     = 65536;

  namespace vk {

    static constexpr VkFormat DepthFormat = VK_FORMAT_D32_SFLOAT;
    static constexpr VkFormat RenderTargetFormat = VK_FORMAT_R8G8B8A8_UINT;

    class ResourcePoolBuilder;

    // old plan
    class ResourcePool {
    public:
      using ResourceHandle = u32;

//      struct BufferHandle {
//        u32 handle : 28;
//        // no more than 2^4 types allowed (ie 16)
//        u32 type_bits : 4;
//
//        BufferHandle operator+(const u32 rhs) {
//          return BufferHandle{handle + rhs, type_bits};
//        }
//
//        const gpu::BufferType get_buffer_type() {
//          return (const gpu::BufferType)type_bits;
//        }
//
//        operator ResourceHandle() {
//          return (ResourceHandle)(handle << 4) | type_bits;
//        }
//      };

      using BufferHandle = u32;
//     struct ImageHandle {
//       u32 handle : 28;
//       // no more than 2^4 types allowed (ie 16)
//       u32 type_bits : 4;
//       ImageHandle operator+(const u32 rhs) {
//         return ImageHandle{handle + rhs, type_bits};
//       }

//       const gpu::ImageType get_image_type() {
//         return (const gpu::ImageType)type_bits;
//       }
//        operator ResourceHandle() {
//          return (ResourceHandle)(handle << 4) | type_bits;
//        }
//      };

      using ImageHandle = u32;
      using SamplerHandle = u32;

      // f's for all of the handle portion of the image handle
      static constexpr ResourceHandle MaxResourceHandle = BIT(28) - 1;

    private:
      friend class ResourcePoolBuilder;
      mem::MemoryPool _memory_pool;

      // buffers
      using buffer_pool_t 
        = Array
          < Buffer               
          >;
      //static constexpr u8 UniformBufferId  = 0;
      //static constexpr u8 StorageBufferId  = 1;
      //static constexpr u8 IndirectBufferId = 2;
      //static constexpr u8 StagingBufferId  = 3;
      // uniform, storage, indirect, staging
      buffer_pool_t _buffer_pool;

      u32 _buffer_arena_ids[6];

      // images
      using image_pool_t 
        = Array
          < Image             
          >;
      using sampler_pool_t
        = Array
          < VkSampler
          >;
      //static constexpr u8 TextureId = 0;
      //static constexpr u8 InputAttachmentId = 1;
      //static constexpr u8 DepthBufferId = 2;
      //static constexpr u8 RenderTargetId = 3;
      //static constexpr u8 StorageImageId = 4;
      // texture, input attachments, depth buffers, render targets, storage images
      image_pool_t _image_pool;
      sampler_pool_t _samplers;

      u32 _image_arena_ids[5];

      Context* _context = nullptr;

      //static const u8 get_buffer_pool_id(const gpu::BufferType type) {
      //  static u8 _s_buffer_type_indices[] = 
      //  { UniformBufferId
      //  , UniformBufferId
      //  , StorageBufferId
      //  , StorageBufferId
      //  , StagingBufferId
      //  , IndirectBufferId
      //  };
      //  return _s_buffer_type_indices[(u32)type];
      //}

      //static const u8 get_image_pool_id(const gpu::ImageType type) {
      //  static u8 _s_image_type_indices[] =
      //  { TextureId
      //  , InputAttachmentId
      //  , RenderTargetId
      //  , DepthBufferId
      //  , StorageImageId
      //  };
      //  return _s_image_type_indices[(u32)type];
      //}

      //buffer_pool_t& get_buffer_pool(const gpu::BufferType type) {
      //  return _buffer_pools[get_buffer_pool_id(type)];
      //}

      //const buffer_pool_t& get_buffer_pool(const gpu::BufferType type) const {
      //  return _buffer_pools[get_buffer_pool_id(type)];
      //}

      u32 get_buffer_arena_id(const gpu::BufferType type) const {
        return _buffer_arena_ids[(u8)type];
      }

      //const image_pool_t& get_image_pool(const gpu::ImageType type) const {
      //  return _image_pools[get_image_pool_id(type)];
      //}

      //image_pool_t& get_image_pool(const gpu::ImageType type) {
      //  return _image_pools[get_image_pool_id(type)];
      //}

      u32 get_image_arena_id(const gpu::ImageType type) const {
        return _image_arena_ids[(u8)type];
      }

      void clear_arenas() {
        const u32 arena_count = _memory_pool.get_arena_count();
        for(u32 i = 0; i < arena_count; i++) {
          _memory_pool.clear_arena(i);
        }
      }

    public:

      b8 init(Context* context, const u32 buffer_info_count, const gpu::ResourceInfo* buffer_infos, const u32 image_info_count, const gpu::ResourceInfo* image_infos);

      Buffer& obtain_buffer(const BufferHandle handle);

      Image& obtain_image(const ImageHandle handle);

      const SamplerHandle create_sampler(const float max_anisotropy = 0.f, const VkFilter min_filter = VK_FILTER_NEAREST, const VkFilter mag_filter = VK_FILTER_NEAREST, const float min_lod = 1.f, const float max_lod = 1.f, const float mip_lod_bias = 1.f, const VkSamplerMipmapMode mip_map_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR, const VkSamplerAddressMode address_mode_u = VK_SAMPLER_ADDRESS_MODE_REPEAT, const VkSamplerAddressMode address_mode_v = VK_SAMPLER_ADDRESS_MODE_REPEAT, const VkSamplerAddressMode address_mode_w = VK_SAMPLER_ADDRESS_MODE_REPEAT, const VkCompareOp compare_op = VK_COMPARE_OP_NEVER);

      VkSampler obtain_sampler(const SamplerHandle handle) const {
        return _samplers[handle];
      }

      void clear_buffer_pool();

      void clear_image_pool();

      void clear_pools();

      void clear();

      b8 terminate();

    };

//    class ResourcePoolBuilder {
//    private:
//      Context* _context;
//
//      mem::MemoryPoolBuilder _memory_builder{};
//      u32 _host_uniform_buffer_mem_id = 0;
//      u32 _device_uniform_buffer_mem_id = 0;
//      u32 _host_storage_buffer_mem_id = 0;
//      u32 _device_storage_buffer_mem_id = 0;
//      u32 _staging_buffer_mem_id = 0;
//      u32          _texture_mem_id = 0;
//      u32 _input_attachment_mem_id = 0;
//      u32    _render_target_mem_id = 0;
//      u32     _depth_target_mem_id = 0;
//      u32     _indirect_buffer_mem_id = 0;
//      u32     _storage_image_mem_id = 0;
//
//      u32 _host_uniform_buffer_count;
//      u32 _device_uniform_buffer_count;
//      u32 _host_storage_buffer_count;
//      u32 _device_storage_buffer_count;
//      u32 _indirect_buffer_count;
//      u32 _staging_buffer_count;
//      u32 _texture_count;
//      u32 _input_attachment_count;
//      u32 _render_target_count;
//      u32 _depth_target_count;
//      u32 _storage_image_count;
//
//
//    public:
//      b8 init(Context* context) {
//        _memory_builder.init(context);
//        return true;
//      }
//
//      void add_resource(gpu::ResourceInfo resource_info = {}) {
//        return create_resource_type(std::move(resource_info));
//      }
//
//      b8 build(Context* context, ResourcePool* pool) {
//        pool->_context = context;
//        _memory_builder.build(&pool->_memory_pool);
//
//        pool->_buffer_arena_ids[ResourcePool::UniformBufferId] = _host_uniform_buffer_mem_id;
//        _device_uniform_buffer_arena_id = _device_uniform_buffer_mem_id;
//        _host_storage_buffer_arena_id = _host_storage_buffer_mem_id;
//        _device_storage_buffer_arena_id = _device_storage_buffer_mem_id;
//        _input_attachment_arena_id = _input_attachment_mem_id;
//        _render_target_arena_id = _render_target_mem_id;
//        _depth_buffer_arena_id = _depth_target_mem_id;
//        _indirect_buffer_arena_id = _indirect_buffer_mem_id;
//        _staging_buffer_arena_id = _staging_buffer_mem_id;
//        _storage_image_arena_id = _storage_image_mem_id;
//        
//        pool->_buffer_pools[ResourcePool::UniformBufferId].move_ptr(ALLOCATE(sizeof(Buffer) * (_host_uniform_buffer_count + _device_uniform_buffer_count)));
//        pool->_buffer_pools[ResourcePool::StorageBufferId].move_ptr(ALLOCATE(sizeof(Buffer) * (_host_storage_buffer_count + _device_storage_buffer_count)));
//        pool->_buffer_pools[ResourcePool::StagingBufferId].move_ptr(ALLOCATE(sizeof(Buffer) * _staging_buffer_count));
//        pool->_image_pools[ResourcePool::TextureId].move_ptr(ALLOCATE(sizeof(Image<2>) * _texture_count));
//        pool->_samplers.move_ptr(ALLOCATE(sizeof(VkSampler) * _texture_count));
//        pool->_image_pools[ResourcePool::RenderTargetId].move_ptr(ALLOCATE(sizeof(Image<2>) * _render_target_count));
//        pool->_image_pools[ResourcePool::DepthBufferId].move_ptr(ALLOCATE(sizeof(Image<2>) * _depth_target_count));
//        pool->_image_pools[ResourcePool::InputAttachmentId].move_ptr(ALLOCATE(sizeof(Image<2>) * _input_attachment_count));
//        pool->_image_pools[ResourcePool::StorageImageId].move_ptr(ALLOCATE(sizeof(Image<2>) * _storage_image_count));
//
//        RX_CHECK
//        ( pool->_buffer_pools[ResourcePool::UniformBufferId].get_buffer() != nullptr
//        , "failed to allocate uniform buffers");
//        RX_CHECK
//        ( pool->_buffer_pools[ResourcePool::StorageBufferId].get_buffer() != nullptr
//        , "failed to allocate storage buffers");
//        RX_CHECK
//        ( pool->_buffer_pools[ResourcePool::StagingBufferId].get_buffer() != nullptr
//        , "failed to allocate staging buffers");
//        RX_CHECK
//        ( pool->_image_pools[ResourcePool::TextureId].get_buffer() != nullptr
//        , "failed to allocate textures");
//        RX_CHECK
//        ( pool->_samplers.get_buffer() != nullptr
//        , "failed to allocate samplers");
//        RX_CHECK
//        ( pool->_image_pools[ResourcePool::RenderTargetId].get_buffer() != nullptr
//        , "failed to allocate render targets");
//        RX_CHECK
//        ( pool->_image_pools[ResourcePool::DepthBufferId].get_buffer() != nullptr
//        , "failed to allocate depth buffers");
//        RX_CHECK
//        ( pool->_image_pools[ResourcePool::InputAttachmentId].get_buffer() != nullptr
//        , "failed to allocate input attachments");
//        RX_CHECK
//        ( pool->_image_pools[ResourcePool::StorageImageId].get_buffer() != nullptr
//        , "failed to allocate storage images");
//        return true;
//      }
//
//      b8 terminate() {
//        _memory_builder.terminate();
//        _context = nullptr;
//        return true;
//      }
//
//    private:
//      void create_resource_type(gpu::ResourceInfo&& info) {
//        switch(info.type) { 
//          case(gpu::ResourceType::InputAttachment): {
//            _input_attachment_count++;
//            const VkMemoryPropertyFlags memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
//            _input_attachment_mem_id
//              = _memory_builder.register_bucket(info.image.depth * info.image.height * info.image.width, memory_flags);
//            return;
//        }
//          case(gpu::ResourceType::RenderTarget): {
//          _render_target_count++;
//          VkMemoryPropertyFlags memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
//          _render_target_mem_id = _memory_builder.register_bucket(info.image.depth * info.image.height * info.image.width, memory_flags);
//          return;
//        }
//          case(gpu::ResourceType::DepthTarget): { 
//          _depth_target_count++;
//          VkMemoryPropertyFlags memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
//          _depth_target_mem_id = _memory_builder.register_bucket(info.image.depth * info.image.height * info.image.width, memory_flags);
//          return;
//        }
//          case(gpu::ResourceType::HostUniformBuffer): {
//          _host_uniform_buffer_count++;
//          VkMemoryPropertyFlags memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
//          _host_uniform_buffer_mem_id = _memory_builder.register_bucket(info.buffer.size, memory_flags);
//          return;
//        }
//          case(gpu::ResourceType::DeviceUniformBuffer): {
//          _device_uniform_buffer_count++;
//          VkMemoryPropertyFlags memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
//          _device_uniform_buffer_mem_id = _memory_builder.register_bucket(info.buffer.size, memory_flags);
//          return;
//        }
//          case(gpu::ResourceType::HostStorageBuffer): {
//          _host_storage_buffer_count++;
//          VkMemoryPropertyFlags memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
//          _host_storage_buffer_mem_id = _memory_builder.register_bucket(info.buffer.size, memory_flags);
//          return;
//        }
//          case(gpu::ResourceType::DeviceStorageBuffer): {
//          _device_storage_buffer_count++;
//          VkMemoryPropertyFlags memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
//          _device_storage_buffer_mem_id = _memory_builder.register_bucket(info.buffer.size, memory_flags);
//          return;
//        }
//          case(gpu::ResourceType::Texture): {
//          _texture_count++;
//          VkMemoryPropertyFlags memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
//          _texture_mem_id = _memory_builder.register_bucket(info.image.depth * info.image.height * info.image.width, memory_flags);
//          return;
//        }
//          case(gpu::ResourceType::IndirectCommand): {
//          _indirect_buffer_count++;
//          VkMemoryPropertyFlags memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
//          _indirect_buffer_mem_id = _memory_builder.register_bucket(info.buffer.size, memory_flags);
//          return;
//        }
//          case(gpu::ResourceType::StagingBuffer): {
//          _staging_buffer_count++;
//          VkMemoryPropertyFlags memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
//          _staging_buffer_mem_id = _memory_builder.register_bucket(info.buffer.size, memory_flags);
//          return;
//        }
//          case(gpu::ResourceType::StorageImage): {
//          _storage_image_count++;
//          VkMemoryPropertyFlags memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
//          _storage_image_mem_id = _memory_builder.register_bucket(info.buffer.size, memory_flags);
//          return;
//        }
//        default:
//          return;
//        }
//      }
//
//    };
  }		// -----  end of namespace vk  ----- 
}		// -----  end of namespace roxi  ----- 
