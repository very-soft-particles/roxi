// =====================================================================================
//
//       Filename:  vk_device.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-05-01 10:50:52 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "vk_device.h"
#include "rx_allocator.hpp"
#include <vulkan/vulkan_core.h>


namespace roxi {
  
  namespace vk {
    b8 Device::terminate() {
      _device_function_table.vkDestroyDevice(_device, CALLBACKS());
      if(_device_queue_indices.get_buffer() != nullptr) {
        FREE(_device_queue_indices.get_buffer());
      }
      return true;
    }

    const VkDevice& Device::get_device() const {
      return _device;
    }

    b8 DeviceBuilder::build(Device* device) {

      RX_TRACE("finding physical device");
      if(!find_physical_device()) {
        RX_FATAL("failed to find appropriate physical device");
        return false;
      }

      RX_TRACE("getting device extensions to use");
      StringList extensions = get_extensions_to_use(); 

      RX_TRACE("getting device layers to use");
      StringList layers = get_layers_to_use();

      VkPhysicalDeviceVulkan12Features vulkan_12_features{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
      vulkan_12_features.timelineSemaphore = VK_TRUE;
      vulkan_12_features.descriptorIndexing = VK_TRUE;
      vulkan_12_features.bufferDeviceAddress = VK_TRUE;

      VkPhysicalDeviceVulkan13Features vulkan_13_features{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
      vulkan_13_features.synchronization2 = VK_TRUE;
      vulkan_13_features.pNext = &vulkan_12_features;

      VkPhysicalDeviceDescriptorBufferFeaturesEXT descriptor_buffer_features{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT};
      descriptor_buffer_features.descriptorBuffer = VK_TRUE;
      descriptor_buffer_features.pNext = &vulkan_13_features;

      VkPhysicalDeviceFeatures2 features{};

      features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
      features.pNext = &descriptor_buffer_features;


      RX_TRACE("getting physical device features");
      _instance->get_instance_function_table().vkGetPhysicalDeviceFeatures2(_chosen_physical_device, &features);

      device->_physical_device = _chosen_physical_device;
      RX_CHECK((device->_physical_device != VK_NULL_HANDLE), "no vulkan physical device could fit chosen settings");

      VkDeviceCreateInfo create_info{};
      Array<char*> extension_array{scratch.push(sizeof(char*) * extensions.node_count)};
      Array<char*> layer_array{scratch.push(sizeof(char*) * layers.node_count)};

      for(auto node = extensions.first;
          node != nullptr;
          node = node->next) {
        *(extension_array.push(1)) = (char*)node->string.str;
      }
      for(auto node = layers.first;
          node != nullptr;
          node = node->next) {
        *(layer_array.push(1)) = (char*)node->string.str;
      }

      RX_TRACE("getting device queue infos");
      Array<VkDeviceQueueCreateInfo> queue_infos = get_queue_create_infos();

      create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
      create_info.pNext = &descriptor_buffer_features;
      create_info.flags = 0;

      create_info.enabledExtensionCount = static_cast<u32>(extension_array.get_size());
      create_info.ppEnabledExtensionNames = (char**)(extension_array.get_buffer());
      create_info.enabledLayerCount = static_cast<u32>(layer_array.get_size());
      create_info.ppEnabledLayerNames = (char**)(layer_array.get_buffer());
      create_info.pEnabledFeatures = &(features.features);
      create_info.pQueueCreateInfos = queue_infos.get_buffer();
      create_info.queueCreateInfoCount = queue_infos.get_size();

      RX_TRACE("creating VkDevice");
      VK_CHECK((_instance->get_instance_function_table().vkCreateDevice(device->_physical_device, &create_info, CALLBACKS(), &device->_device)), "failed to create VkDevice");

      RX_TRACE("loading device level functions");
#define DEVICE_LEVEL_VULKAN_FUNCTION( name )                                \
    device->_device_function_table.name =                                   \
      (PFN_##name)vkGetDeviceProcAddr( device->_device, #name );            \
    if( device->_device_function_table.name == nullptr ) {                  \
      LOG("Could not load instance level Vulkan function named: "           \
        #name, Warn);                                                       \
      return false;                                                         \
    }

#include "vk_function_list.inl"

      RX_TRACE("loading device level functions with extensions");
#define DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( name, extension )      \
    for(const StringNode* node = extensions.first;                          \
        node != nullptr;                                                    \
        node = node->next) {                                                \
      if(str_compare(node->string, lofi::str_lit(extension))) {             \
        device->_device_function_table.name =                               \
        (PFN_##name)vkGetDeviceProcAddr(device->_device, #name );           \
      }                                                                     \
    }                                                                       \
    if( device->_device_function_table.name == nullptr ) {                  \
      LOG("Could not load device level Vulkan function named: "             \
      #name, Warn);                                                         \
      return false;                                                         \
    }                                  

#include "vk_function_list.inl"

      RX_TRACE("getting device queues");
      const auto queue_count = _chosen_queue_indices.get_size();
      device->_device_queue_indices.move_ptr(ALLOCATE(sizeof(DeviceQueueIndex) * queue_count));
      for(size_t i = 0; i < queue_count; i++) {
        device->_device_queue_indices[i] = _chosen_queue_indices[i];
      }

      RX_TRACE("returning from DeviceBuilder::build(Device*)");
      return true;
    }

    VkPhysicalDevice Device::get_physical_device() const {
      return _physical_device;
    }

    DeviceQueueIndex Device::get_queue_family_index(const u32 queue_index) const {
       return _device_queue_indices[queue_index];
    }

    VkPhysicalDevice DeviceBuilder::get_physical_device() {
      if(_chosen_physical_device == VK_NULL_HANDLE) {
        find_physical_device();
      }
      return _chosen_physical_device;
    }

    const Device::FunctionTable& Device::get_device_function_table() const {
      return _device_function_table;
    }


    b8 DeviceBuilder::find_physical_device() {

      uint32_t count = 0;
      _instance->get_instance_function_table().vkEnumeratePhysicalDevices(_instance->get_instance(), &count, nullptr);

      const u32 physical_devices_allocation_size = sizeof(VkPhysicalDevice) * count;
      const u32 scores_allocation_size = sizeof(float) * count;
      Array<VkPhysicalDevice> device_list{scratch.push(physical_devices_allocation_size)};
      Array<float> scores_list{scratch.push(scores_allocation_size)};
      scores_list.push(count);

      _instance->get_instance_function_table().vkEnumeratePhysicalDevices(_instance->get_instance(), &count, device_list.push(count));

      u64 max = MAX_u64;
      for(size_t i = 0; i < count; i++) {
        VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_props;
        descriptor_buffer_props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT;
        descriptor_buffer_props.pNext = nullptr;

        VkPhysicalDeviceProperties2 props;
        props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        props.pNext = &descriptor_buffer_props;

        RX_TRACEF("getting physical device props for device at %llu", i);
        _instance->get_instance_function_table().vkGetPhysicalDeviceProperties2(device_list[i], &props);
        scores_list[i] += props.properties.limits.bufferImageGranularity;
        if(props.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) { 
          scores_list[i] += 1000;
        } else if (props.properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
          scores_list[i] = 0;
        }
        if(max != MAX_u64) {
          if(scores_list[i] > scores_list[max]) {
            max = i;
          }
        } else {
          max = i;
        }
      }
      if(max == MAX_u64) {
        return false;
      }
      _chosen_physical_device = device_list[max];
      scratch.pop_amount(physical_devices_allocation_size + scores_allocation_size);
      return true;
    }

    StringList DeviceBuilder::get_available_extensions() {
      StringList available_extensions;

      u32 count = 0;

      _instance->get_instance_function_table().vkEnumerateDeviceExtensionProperties(
          _chosen_physical_device, nullptr, &count, nullptr);

      Array<VkExtensionProperties> ext_props(scratch.push(sizeof(VkExtensionProperties) * count));

      MEM_ZERO(ext_props.get_buffer(), ext_props.get_size() * sizeof(VkExtensionProperties));

      _instance->get_instance_function_table().vkEnumerateDeviceExtensionProperties(
          _chosen_physical_device, nullptr, &count, ext_props.push(count));

      for(size_t i = 0; i < count; i++) {
        String str = lofi::str_cstring(ext_props[i].extensionName);
        str_list_push(&scratch, &available_extensions, str);
      }
      return available_extensions;
    }

    StringList DeviceBuilder::get_available_layers() {
      StringList available_layers;
      u32 count = 0;

      _instance->get_instance_function_table().vkEnumerateDeviceLayerProperties(
            _chosen_physical_device, &count, nullptr);
 
      Array<VkLayerProperties> layer_props{scratch.push(sizeof(VkLayerProperties) * count)};
 
      _instance->get_instance_function_table().vkEnumerateDeviceLayerProperties(
            _chosen_physical_device, &count, layer_props.push(count));

      for(size_t i = 0; i < count; i++) {
        String str = lofi::str_cstring(layer_props[i].layerName);
        str_list_push(&scratch, &available_layers, str);
      }

      return available_layers;
    }

    StringList DeviceBuilder::get_extensions_to_use() {
      StringList avail_exts = get_available_extensions();

      StringList chosen_extensions{};

      for(const StringNode* nodei = _enabled_extensions.first;
          nodei != nullptr;
          nodei = nodei->next) {
        String string = nodei->string;
        for(const StringNode* nodej = avail_exts.first;
            nodej != nullptr;
            nodej = nodej->next) {
          if(str_compare(string, nodej->string)) {
            str_list_push(&scratch, &chosen_extensions, string);
            break;
          }
        }
      }
      
      return chosen_extensions;
    }

    StringList DeviceBuilder::get_layers_to_use() {
      StringList chosen_layers{};
      StringList avail_layers = get_available_layers();
      for(const StringNode* nodei = _enabled_layers.first;
          nodei != nullptr;
          nodei = nodei->next) {
        String string = nodei->string;
        for(const StringNode* nodej = avail_layers.first;
            nodej != nullptr;
            nodej = nodej->next) {
          if(str_compare(string, nodej->string)) {
            str_list_push(&scratch, &chosen_layers, string);
          }
        }
      }
      
      return chosen_layers;
    }

    b8 DeviceBuilder::get_available_queue_families() {

      u32 count = 0;
      
      _instance->get_instance_function_table().vkGetPhysicalDeviceQueueFamilyProperties(_chosen_physical_device, &count, nullptr);

      RX_TRACEF("found %u many queue families with current chosen device", count);
      
        _queue_family_props.move_ptr(
          scratch.push(
            count * sizeof(VkQueueFamilyProperties)
          )
        );
      _instance->get_instance_function_table().vkGetPhysicalDeviceQueueFamilyProperties(_chosen_physical_device, &count, _queue_family_props.push(count));

      return true;
    }

    b8 DeviceBuilder::init() {
      scratch.move_ptr(ALLOCATE(KB(64)));
      _p_next_chain.move_ptr(scratch.push(sizeof(void*) * 32));
      // in case no p next is added
      _p_next_chain[0] = nullptr;
      _queue_registrations.move_ptr(scratch.push(sizeof(QueueType) * 32));
      return true;
    }

    b8 DeviceBuilder::terminate() {
      FREE((void*)scratch.get_buffer());
      return true;
    }

    Array<VkDeviceQueueCreateInfo> DeviceBuilder::get_queue_create_infos() {

      Array<VkDeviceQueueCreateInfo> result;

      const auto queue_count = _queue_registrations.get_size();

      RX_TRACE("allocating chosen queue indices");
      _chosen_queue_indices.move_ptr(scratch.push(sizeof(DeviceQueueIndex) * queue_count));
      DeviceQueueIndex* chosen_device_queues_begin = _chosen_queue_indices.push(queue_count);
      MEM_FILL_ONES(chosen_device_queues_begin, sizeof(DeviceQueueIndex) * queue_count);

      RX_TRACE("getting available queue families");
      RX_RETURN(get_available_queue_families(), "failed to find available queue families", result);

      const u64 queue_family_count = _queue_family_props.get_size();

      u32 queue_indices[(u8)QueueType::Max];
      MEM_FILL_ONES_ARRAY(queue_indices);

      const u32 queue_counts_size = sizeof(u32) * queue_family_count;
      Array<u32> queue_counts{scratch.push(queue_counts_size)};
      queue_counts.push(queue_family_count);
      MEM_ZERO(queue_counts.get_buffer(), queue_counts_size);

      RX_TRACE("finding queues");
      for(u64 i = 0; i < queue_family_count; i++) {
        VkBool32 present_support = VK_FALSE;
        const VkQueueFamilyProperties& props = _queue_family_props[i];
        _instance->get_instance_function_table()
          .vkGetPhysicalDeviceSurfaceSupportKHR
            ( _chosen_physical_device
            , (u32)i
            , _chosen_surface
            , &present_support
            );
        RX_TRACEF("checking queue family index %llu, queue_flags = %x, present_support = %d", i, _queue_family_props[i].queueFlags, (b8)present_support);
        if
          (check_queue_reqs_by_type<QueueType::Generic>(props) 
          && (queue_indices[(u32)QueueType::Generic] == MAX_u32)
          && present_support == VK_TRUE
          ) {
          RX_TRACE("found generic queue type");
          queue_indices[(u32)QueueType::Generic] = i;
        }
        if
          (check_queue_reqs_by_type<QueueType::Render>(props) 
          && (queue_indices[(u32)QueueType::Render] == MAX_u32)
          && present_support == VK_TRUE
          ) {
          RX_TRACE("found render queue type");
          queue_indices[(u32)QueueType::Render] = i;
        }
        if
          (check_queue_reqs_by_type<QueueType::Compute>(props) 
          && (queue_indices[(u32)QueueType::Compute] == MAX_u32) 
          ) {
          RX_TRACE("found compute queue type");
          queue_indices[(u32)QueueType::Compute] = i;
        }
        if
          (check_queue_reqs_by_type<QueueType::Transfer>(props) 
          && (queue_indices[(u32)QueueType::Transfer] == MAX_u32) 
          ) {
          RX_TRACE("found transfer queue type");
          queue_indices[(u32)QueueType::Transfer] = i;
        }
      }

      RX_TRACE("setting queue infos");
      const u32 generic_queue_family_index = queue_indices[(u8)QueueType::Generic];
      for(u64 i = 0; i < queue_count; i++) {
        const u32 queue_type_id = (u32)(_queue_registrations[i]);
        const u32 queue_family_index = queue_indices[queue_type_id];
        chosen_device_queues_begin[i].queue_family_index = queue_family_index;
        chosen_device_queues_begin[i].queue_id = queue_counts[queue_family_index]++;
        RX_TRACEF("queue_family_index = %u at queue_index %llu, queue_id = %u", queue_family_index, i, chosen_device_queues_begin[i].queue_id);
        if(queue_counts[queue_family_index] <= _queue_family_props[queue_family_index].queueCount) {
          continue;
        }
        chosen_device_queues_begin[i].queue_family_index = generic_queue_family_index;
        chosen_device_queues_begin[i].queue_id = queue_counts[generic_queue_family_index]++;
        RX_TRACEF("queue_family_index changed to generic at %u, queue_id = %u", generic_queue_family_index, chosen_device_queues_begin[i].queue_id);

        RX_RETURNF((queue_counts[generic_queue_family_index] < _queue_family_props[generic_queue_family_index].queueCount), result, "failed to create roxi::vk::QueueType::%s at index %u", get_queue_type_string(_queue_registrations[i]), i);
      }

      RX_TRACE("allocating queue priorities");
      Array<float> priorities(scratch.push(sizeof(float) * (u8)QueueType::Max));
          
      RX_TRACE("allocating queue create infos");
      result.move_ptr(scratch.push(sizeof(VkDeviceQueueCreateInfo) * queue_count));

      for(u32 i = 0; i < queue_count; i++) {
        const u32 queue_type_id = (u32)(_queue_registrations[i]);
        const u32 queue_family_index = queue_indices[queue_type_id];
        const u32 queues_in_family_count = queue_counts[queue_family_index];

        if(queues_in_family_count == 0) {
          RX_TRACEF("no queues at %u... continuing", i);
          continue;
        }

        RX_TRACEF("queue count at %u = %u", i, queue_count);
        float* p_priorities = priorities.push(queues_in_family_count);
        for(u32 j = 0; j < queues_in_family_count; j++) {
          p_priorities[j] = 1.f;
        }
       
        VkDeviceQueueCreateInfo& create_info = *(result.push(1));
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;
        create_info.queueFamilyIndex = i;
        create_info.queueCount = queues_in_family_count;
        create_info.pQueuePriorities = p_priorities;
      }

      return result;
    }

    void DeviceBuilder::set_extensions(StringList* new_extensions) {
      for(StringNode* node = new_extensions->first;
          node != nullptr;
          node = node->next) {
        str_list_push(&scratch, &_enabled_extensions, node->string);
      }
    }

    void DeviceBuilder::set_layers(StringList* new_layers) {
      for(StringNode* node = new_layers->first;
          node != nullptr;
          node = node->next) {
        str_list_push(&scratch, &_enabled_layers, node->string);
      }
    }

    void DeviceBuilder::set_surface(VkSurfaceKHR surface) {
      _chosen_surface = surface;
    }

    void DeviceBuilder::add_extension(const char* new_extension) {
      String string = lofi::str_cstring(new_extension);
      str_list_push(&scratch, &_enabled_extensions, string);
    }

    void DeviceBuilder::add_layer(const char* new_layer) {
      String string = lofi::str_cstring(new_layer);
      str_list_push(&scratch, &_enabled_layers, string);
    }
  }		// -----  end of namespace vk  ----- 
}		// -----  end of namespace roxi  ----- 
