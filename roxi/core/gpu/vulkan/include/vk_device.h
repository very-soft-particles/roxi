// =====================================================================================
//
//       Filename:  vk_device.h
//
//    Description:  
//
//        Version:  1.0
//        Created:  2023-11-27 3:42:29 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "vk_instance.h"

#define DEVICE_QUEUE_TYPES(X) X(Generic) X(Render) X(Compute) X(Transfer) X(Max)

namespace roxi {
  namespace vk {

    enum class QueueType : u32 {
#define ENUM(X) X,
      DEVICE_QUEUE_TYPES(ENUM)
#undef ENUM
    };

    static const char* get_queue_type_string(const QueueType type) {
#define FIND_STR(X) if(type == QueueType::X) return STRINGIFY(QueueType::X);
      DEVICE_QUEUE_TYPES(FIND_STR)
#undef FIND_STR
        return "Unknown";
    }

    template<QueueType Type>
    static b8 check_queue_reqs_by_type(const VkQueueFamilyProperties& props) {
      if constexpr (Type == QueueType::Generic) {

        return (props.queueFlags 
             & (VK_QUEUE_GRAPHICS_BIT 
              | VK_QUEUE_COMPUTE_BIT 
              | VK_QUEUE_TRANSFER_BIT)) 
            == (VK_QUEUE_GRAPHICS_BIT 
              | VK_QUEUE_COMPUTE_BIT 
              | VK_QUEUE_TRANSFER_BIT);
      } else if constexpr (Type == QueueType::Render) {

        return (props.queueFlags 
             & ( VK_QUEUE_GRAPHICS_BIT 
              | VK_QUEUE_COMPUTE_BIT
              | VK_QUEUE_TRANSFER_BIT)) 
            == ( VK_QUEUE_GRAPHICS_BIT 
              | VK_QUEUE_COMPUTE_BIT
              | VK_QUEUE_TRANSFER_BIT);
      } else if constexpr (Type == QueueType::Compute) {

        return (props.queueFlags & VK_QUEUE_COMPUTE_BIT 
          && !(props.queueFlags & VK_QUEUE_GRAPHICS_BIT));
      } else if constexpr (Type == QueueType::Transfer) {

        return props.queueFlags & VK_QUEUE_TRANSFER_BIT 
          && !(props.queueFlags & VK_QUEUE_COMPUTE_BIT)
          && !(props.queueFlags & VK_QUEUE_GRAPHICS_BIT);
      }
      return false;
    }

    struct DeviceQueueIndex {
      u32 queue_family_index = MAX_u32;
      u32 queue_id = MAX_u32;
    };

    class DeviceBuilder;

    class Device {
    private:
      friend class DeviceBuilder;
    public:
      // + 1 queue for the ALL queue
      struct FunctionTable {

#define DEVICE_LEVEL_VULKAN_FUNCTION( name ) PFN_##name name = nullptr;
#define DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( name, extension ) PFN_##name name = nullptr;
#include "vk_function_list.inl"
#undef DEVICE_LEVEL_VULKAN_FUNCTION
#undef DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION

      };

    private:
      VkPhysicalDevice _physical_device = VK_NULL_HANDLE;
      VkDevice _device = VK_NULL_HANDLE; 
      FunctionTable _device_function_table;
      Array<DeviceQueueIndex> _device_queue_indices;
    public:
      b8 terminate();

      const VkQueue get_queue(const u32 handle) const {
        VkQueue result = VK_NULL_HANDLE;
        _device_function_table.vkGetDeviceQueue(_device, _device_queue_indices[handle].queue_family_index, _device_queue_indices[handle].queue_id, &result);
        return result;
      }

      const VkQueue get_queue(const u32 handle) {
        VkQueue result = VK_NULL_HANDLE;
        _device_function_table.vkGetDeviceQueue(_device, _device_queue_indices[handle].queue_family_index, _device_queue_indices[handle].queue_id, &result);
        return result;
      }

      VkPhysicalDevice get_physical_device() const;
      // return const ref so that you don't have to copy the VkDevice in order to get a pointer
      // because ptr to r value is invalid
      const VkDevice& get_device() const;
      const FunctionTable& get_device_function_table() const;

      DeviceQueueIndex get_queue_family_index(const u32 queue_index) const;

    private:
    };

    class DeviceBuilder {
    public:
      // queue family index, index of queue in queue family
      // + 1 queue for the ALL queue
    private:      
      Arena scratch;
      VkPhysicalDevice _chosen_physical_device = VK_NULL_HANDLE;
      Array<VkQueueFamilyProperties> _queue_family_props;
      Array<DeviceQueueIndex> _chosen_queue_indices;
      Array<QueueType>         _queue_registrations;
      Array<void*> _p_next_chain;
      Instance* _instance;
      StringList _enabled_extensions;
      StringList _enabled_layers;
      VkSurfaceKHR _chosen_surface = VK_NULL_HANDLE;

      VkPhysicalDevice get_physical_device();
      Array<QueueType> get_queue_indices();

      StringList get_extensions_to_use();
      StringList get_layers_to_use();
    public:
      b8 init();

      b8 terminate();

      void set_instance(Instance* instance) {
        _instance = instance;
      }

      void set_extensions(StringList* new_extension_names);
      void set_layers(StringList* new_layer_names);

      void set_surface(VkSurfaceKHR surface);

      void add_extension(const char* new_extension);
      void add_layer(const char* new_layer);

      void add_p_next(void* next) {
        const u32 p_next_size = _p_next_chain.get_size();
        *(_p_next_chain.push(1)) = (void*)next;
        if(p_next_size > 0) {
          u8* p_next = ((u8*)(_p_next_chain[p_next_size - 1])) + sizeof(VkStructureType);
          *(void**)p_next = next;
        }
      }

      const u32 register_queue(const QueueType type) {
        const u32 result = (const u32)_queue_registrations.get_size();
        *(_queue_registrations.push(1)) = type;
        return result;
      }

      b8 build(Device* device);

    private:
      b8 find_physical_device();
      Array<VkDeviceQueueCreateInfo> get_queue_create_infos();
      b8 get_available_queue_families();
      StringList get_available_extensions();
      StringList get_available_layers();

    };
 
  }		// -----  end of namespace vk  ----- 
}		// -----  end of namespace roxi  ----- 
