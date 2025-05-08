// =====================================================================================
//
//       Filename:  rx_rendergraph.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-09-10 10:04:25 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "../include/rx_rendergraph.hpp"
#include "rx_rendergraph.hpp"
#include "vk_resource.hpp"
#include <vcruntime.h>

namespace roxi {

  b8 GPUGraph::terminate() {
    const size_t num_nodes = nodes.get_size();
    for(size_t i = 0; i < num_nodes; i++) {
     FREE((void*)nodes[i].inputs.get_buffer());
    }
    FREE((void*)node_names.get_buffer());
    return true;
  }

  b8 GPUGraphBuilder::init(GPUGraph* rendergraph) {
    _graph = rendergraph;
    u8* buffer = (u8*)ALLOCATE
      ( sizeof(char*) * MaxGPUGraphNodes
      + sizeof(char*) * MaxGPUGraphNodes
      + sizeof(GPUGraphNode) * MaxGPUGraphNodes
      + sizeof(GPUGraphResource) * MaxGPUGraphNodes
      );
    if(!buffer) {
      LOG("failed to allocate GPUGraph", Error);
      return false;
    }
    _graph->node_names.move_ptr(buffer);
    buffer += sizeof(char*) * MaxGPUGraphNodes;

    _graph->resource_names.move_ptr(buffer);
    buffer += sizeof(char*) * MaxGPUGraphNodes;

    _graph->nodes.move_ptr(buffer);
    buffer += sizeof(GPUGraphNode) * MaxGPUGraphNodes;

    _graph->resources.move_ptr(buffer);

    return true;
  }

  u32 GPUGraphBuilder::add_resource(const char* name, gpu::ResourceInfo info, u32 base_resource_id) {
    const u32 id = _graph->resources.get_size();
    *(_graph->resource_names.push(1)) = lofi::str_cstring(name);
    GPUGraphResource* resource = _graph->resources.push(1);
    resource->id = id;
    resource->info = info;
    if(base_resource_id != MAX_u32) {
      resource->base_resource_id = base_resource_id;
    } else {
      resource->base_resource_id = id;
    }
    return id;
  }

  u32 GPUGraphBuilder::add_node(const char* name, u32 num_inputs, u32 num_outputs, u32* input_ids, u32* output_ids) {
    u32 id = _graph->nodes.get_size();
    *(_graph->node_names.push(1)) = lofi::str_cstring(name);
    GPUGraphNode* node = _graph->nodes.push(1);
    node->id = id;

    u8* buffer = (u8*)ALLOCATE(
          num_inputs * sizeof(u32)
        + num_outputs * sizeof(u32)
        + num_inputs * num_outputs * sizeof(u32)
        );

    node->inputs.move_ptr(buffer);
    buffer += num_inputs * sizeof(u32);

    node->outputs.move_ptr(buffer);
    buffer += num_outputs * sizeof(u32);

    node->edges.move_ptr(buffer);

    if(node->inputs.get_buffer() == nullptr || node->outputs.get_buffer() == nullptr) {
      return MAX_u32;
    }


    node->inputs.push(num_inputs);
    node->outputs.push(num_outputs);
    for(size_t i = 0; i < num_inputs; i++) {
      if(_graph->resources[input_ids[i]].info.type == gpu::ResourceType::InputAttachment) {
        _graph->resources[input_ids[i]].producer_id = id;
      }
      node->inputs[i] = input_ids[i];
    }
    for(size_t i = 0; i < num_outputs; i++) {
      node->outputs[i] = output_ids[i];
    }
    return id;
  }

  b8 GPUGraphBuilder::compile() {
    const size_t num_nodes = _graph->nodes.get_size();
    SizedStackArray<u8, MaxGPUGraphNodes> visited{};

    _graph->sorted_nodes.move_ptr(ALLOCATE(num_nodes * sizeof(GPUGraphNodeHandle))); // NOLINT
    SizedStackArray<u32, MaxGPUGraphNodes> stack{};
    MEM_ZERO(visited.push(num_nodes), num_nodes * sizeof(u8));

    // use topological sort for graph nodes
    for(size_t i = 0; i < num_nodes; i++) {
      const size_t num_inputs = _graph->nodes[i].inputs.get_size();
      for(size_t j = 0; j < num_inputs; j++) {
        *(_graph->nodes[
          _graph->resources[_graph->nodes[i].inputs[j]].producer_id
             ].edges.push(1)) = i;
      }
      *(stack.push(1)) = i;
      while(stack.get_size() > 0) {
        u32 handle = stack[stack.get_size() - 1];
        if(visited[handle] == 1) {
          *(_graph->sorted_nodes.push(1)) = &_graph->nodes[handle];
          visited[handle]++;
          stack.pop(1);
          continue;
        }
        if(visited[handle] == 2) {
          stack.pop(1);
          continue;
        }
        visited[handle]++;
        const size_t num_edges = _graph->nodes[handle].edges.get_size();
        if(num_edges == 0) {
          continue;
        }
        for(size_t j = 0; j < num_edges; j++) {
          u32 edge_id = _graph->nodes[i].edges[j];
          if(visited[edge_id] == 0) {
            *(stack.push(1)) = edge_id;
          }
        }
      }
    }

    const size_t num_resources = _graph->resources.get_size();
    visited.clear();
    visited.push(num_resources);                                                             // use visited as ref counter and stack as free list
    MEM_ZERO(visited.get_buffer(), sizeof(u8) * num_resources);

    for(size_t i = 0; i < num_nodes; i++) {
      const size_t num_inputs = _graph->sorted_nodes[i]->inputs.get_size();
      for(size_t j = 0; j < num_inputs; j++) {
        visited[_graph->resources[_graph->sorted_nodes[i]->inputs[j]].base_resource_id]++;                          // increment ref count when resource is used as input
      }
    }

    for(size_t i = 0; i < num_nodes; i++) {
      const size_t num_outputs = _graph->sorted_nodes[i]->outputs.get_size();
      for(size_t j = 0; j < num_outputs; j++) {
        if(_graph->resources[_graph->sorted_nodes[i]->outputs[j]].info.type == gpu::ResourceType::InputAttachment) {
          if(stack.get_size() != 0) {  // use stack as free list
            _graph->resources[_graph->sorted_nodes[i]->outputs[j]].base_resource_id = stack[stack.get_size() - 1];
          }
        }
      }
      const size_t num_inputs = _graph->sorted_nodes[i]->inputs.get_size();
      for(size_t j = 0; j < num_inputs; j++) {
        const u32 base_id = _graph->resources[_graph->sorted_nodes[i]->inputs[j]].base_resource_id;
        if(--(visited[base_id]) == 0) {                                                             // if visited at the base_id = 0, then this is the last node to use this resource and it can be added to the list
          *(stack.push(1)) = base_id;
        }
      }
    }

    return true;
  }

  const GPUGraph* GPUGraphBuilder::get() {
    return _graph;
  }

  String GPUGraphBuilder::get_node_name(u32 id) const {
    return _graph->node_names[id];
  }

  String GPUGraphBuilder::get_resource_name(u32 id) const {
    return _graph->resource_names[id];
  }



}		// -----  end of namespace roxi  ----- 
