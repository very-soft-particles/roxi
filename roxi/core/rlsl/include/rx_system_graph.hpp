// =====================================================================================
//
//       Filename:  rx_system_graph.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-12-03 12:29:35 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "rx_allocator.hpp"
#include "rx_system_graph.hpp"
#include "rx_vocab.h"

namespace roxi {
 
  namespace rlsl {

    static constexpr u32 MaxTaskTypesPerSystem = 128;
    static constexpr u32 MaxTaskResourcesPerSystem = 256;

    enum class TaskPriority {
      High,
      Medium,
      Low
    };

    enum class TaskResourceType {
      Local,
      External,
      Max
    };

    struct TaskResourceInfo {
      union {
        struct {
          u32 size = MAX_u32;
        } local;
        struct {
          u32 component_id = MAX_u32;
        } external;
      };
      TaskResourceType type = TaskResourceType::Max;
    };

    struct TaskResource {
      static constexpr u32 NullProducerId = MAX_u32;
      TaskResourceInfo info;
      u32 producer_id = NullProducerId;
      u32 base_resource_id;
      u32 ref_count = 0;
    };

    struct TaskNode {
      String name;

      Job entry;
      Array<u32> input_ids;
      Array<u32> output_ids;
      Array<u32> edge_ids;
      TaskPriority priority = TaskPriority::Medium;
      u32 level = 0;
    };

    TaskNode create_task_node(const char* name, Job entry, const u32 input_count, const u32 output_count, const u32* input_ids, const u32* output_ids, const TaskPriority priority = TaskPriority::Medium);

    class SystemBuilder;

    class System {
    private:
      friend class SystemBuilder;
      Array<TaskNode> _tasks;
      Array<TaskResource> _resources;
      StackArena _arena{};

   public:

      b8 update();

      b8 terminate();
    };

    class SystemBuilder {
    private:
      Array<TaskResource> _resources{};
      Array<TaskNode> _tasks{};
    public:
      using ResourceHandle = typename Array<TaskResource>::index_t;
      using TaskHandle = typename Array<TaskNode>::index_t;
      b8 init() {
        u8* buffer = (u8*)ALLOCATE
          ( sizeof(TaskNode) * MaxTaskTypesPerSystem
          + sizeof(TaskResource) * MaxTaskResourcesPerSystem
          );
        _tasks.move_ptr(buffer);
        buffer += sizeof(TaskNode) * MaxTaskTypesPerSystem;

        _resources.move_ptr(buffer);
        RX_CHECK(_tasks.get_buffer() != nullptr
          , "failed to allocate task nodes in system builder");
        return true;
      }

      const ResourceHandle add_resource(TaskResource resource) {
        const ResourceHandle result = _resources.get_size();
        *(_resources.push(1)) = resource;
        // resources are always initialized to be their own base resource, then in the build function the
        // sort checks whether a resource can reuse memory from another resource that is no longer needed
        _resources[result].base_resource_id = result;
        return result;
      }

      const TaskHandle add_task(TaskNode task) {
        const TaskHandle result = _tasks.get_size();
        *(_tasks.push(1)) = task;
        return result;
      }

      b8 build(System* system) {
        const u32 node_count = _tasks.get_size();
        // use topological sort for graph nodes
        for(u32 n = 0; n < node_count; n++) {
          const auto& current_task = _tasks[n];
          const u32 output_count = current_task.output_ids.get_size();
          const u32 input_count = current_task.input_ids.get_size();
          // find every output that isn't also an input and 
          // mark this node as the producer of this
          // resource
          for(u32 o = 0; o < output_count; o++) {
            const ResourceHandle out_handle = current_task.output_ids[o];
            b8 read_write = false;
            for(u32 i = 0; i < input_count; i++) {
              const ResourceHandle in_handle = _tasks[n].input_ids[i];
              if(in_handle != out_handle) {
                continue;
              }
              read_write = true;
              break;
            }
            if(!read_write) {
              _resources[out_handle].producer_id = n;
            }
          }
        }

        for(u32 n = 0; n < node_count; n++) {
          auto& current_task = _tasks[n];
          const u32 input_count = current_task.input_ids.get_size();
          // find the producers for each input 
          for(u32 i = 0; i < input_count; i++) {
            const ResourceHandle in_handle = _tasks[n].input_ids[i];
            const TaskHandle producer_handle = _resources[in_handle].producer_id;
            if(producer_handle != TaskResource::NullProducerId) {
              *(current_task.edge_ids.push(1)) = producer_handle;
            }
          }
        }

        Array<u32> sorted_nodes;
        sorted_nodes.move_ptr(ALLOCATE(node_count * sizeof(u32))); // NOLINT
                                                                       //
        SizedStackArray<u8, 64> visited{};
        MEM_ZERO(visited.push(node_count), node_count * sizeof(u8));

        SizedStackArray<u32, 64> stack{};

        for(u32 n = 0; n < node_count; n++) {
          *(stack.push(1)) = n;
          while(stack.get_size() > 0) {
            const u32 current_node_handle = stack[stack.get_size() - 1];
            const auto& current_node = _tasks[current_node_handle];
            const auto current_handle_visited = visited[current_node_handle];
            if(current_handle_visited == 1) {
              *(sorted_nodes.push(1)) = current_node_handle;
              visited[current_node_handle] = 2;
              stack.pop(1);
              continue;
            }
            if(current_handle_visited == 2) {
              stack.pop(1);
              continue;
            }
            visited[current_node_handle] = 1;
            const auto edge_count = current_node.edge_ids.get_size();
            if(edge_count == 0) {
              // if no outgoing edges, it is a termination node (ie a leaf)
              continue;
            }
            for(u32 e = 0; e < edge_count; e++) {
              const u32 edge_id = current_node.edge_ids[e];
              if(visited[edge_id] == 0) {
                // if current edge has not been visited, push to back of stack and process on
                // next pass through the while loop
                *(stack.push(1)) = edge_id;
              }
            }
          }
        }

        const auto resource_count = _resources.get_size();
        visited.clear();
        // use visited as ref counter and stack as free list
        visited.push(resource_count);
        MEM_ZERO(visited.get_buffer(), sizeof(u8) * resource_count);

        stack.clear();

        // loop through sorted nodes this time
        for(u32 n = 0; n < node_count; n++) {
          const auto& current_task = _tasks[sorted_nodes[n]];
          const auto input_count = current_task.input_ids.get_size();
          for(u32 i = 0; i < input_count; i++) {
            const auto& current_input = _resources[current_task.input_ids[i]];
            // increment ref count when resource is used as input
            visited[current_input.base_resource_id]++;
          }
        }

        // loop through sorted nodes a second time
        for(u32 n = 0; n < node_count; n++) {
          const auto& current_task = _tasks[sorted_nodes[n]];
          const size_t output_count = current_task.output_ids.get_size();
          for(u32 o = 0; o < output_count; o++) {
            auto& current_output = _resources[current_task.output_ids[o]];
            if(current_output.info.type == TaskResourceType::Local) {
              const auto stack_size = stack.get_size();
              if(stack_size != 0) {
                // use stack as free list
                const u32 current_size = current_output.info.local.size;
                u32 index = stack_size;
                while(index--) {
                  const ResourceHandle other_handle = stack[index];
                  const u32 other_size = _resources[other_handle].info.local.size;
                  if(current_size <= other_size) {
                    current_output.base_resource_id = other_handle;
                    if(index != (stack_size - 1)) {
                      stack[index] = stack[stack_size - 1];
                    }
                    stack.pop(1);
                    break;
                  }
                }
              }
            }
          }
          const size_t num_inputs = _tasks[sorted_nodes[n]].input_ids.get_size();
          for(size_t i = 0; i < num_inputs; i++) {
            const u32 base_id = _resources[_tasks[sorted_nodes[n]].input_ids[i]].base_resource_id;
            if(--(visited[base_id]) == 0) {
              // if visited at the base_id = 0, then this is the last node to use this resource and it can be added to the list
              *(stack.push(1)) = base_id;
            }
          }
        }

        StackArray<u32> allocations;
        MEM_FILL_ONES(allocations.push(resource_count), resource_count * sizeof(u32));

        StackArray<u32> deallocations;
        MEM_FILL_ONES(deallocations.push(resource_count), resource_count * sizeof(u32));

        stack.clear();

        const u32 task_count = _tasks.get_size();
        u32 current_level_offset = 0;
        while(true) {
          for(u32 i = current_level_offset; i < task_count; i++) {
            auto& task = _tasks[sorted_nodes[i]];
            const u32 input_component_count = task.input_ids.get_size();
            u32 count = 0;
            for(u32 j = 0; j < input_component_count; j++) {
              if(_resources[task.input_ids[j]].info.type == TaskResourceType::Local
                && _resources[task.input_ids[j]].producer_id > current_level_offset) {
                count++;
              }
            }
            if(count != 0) {
              task.level++;
            } else {
              current_level_offset = i;
            }
          }
          if(current_level_offset == task_count - 1) {
            break;
          }
        }

        for(u32 i = 0; i < task_count; i++) {

          auto& task = _tasks[sorted_nodes[i]];

          const u32 output_component_count = task.output_ids.get_size();
          for(u32 j = 0; j < output_component_count; j++) {
            ResourceHandle handle = task.output_ids[j];
            TaskResource& resource = _resources[handle];

            if(resource.info.type != TaskResourceType::External && allocations[handle] == lofi::index_type_max<ResourceHandle>::value) {
              allocations[handle] = sorted_nodes[i];
              const u32 free_list_size = stack.get_size();

              if(free_list_size > 0) {
                const u32 free_resource_handle = stack[free_list_size - 1];
                stack.pop(1);
                const u32 current_resource_size = resource.info.local.size;
                const u32 other_resource_size = _resources[free_resource_handle].info.local.size;

                if(current_resource_size <= other_resource_size) {
                  resource.base_resource_id = free_resource_handle;
                }
              }
            } 
          }

          const u32 input_component_count = task.input_ids.get_size();
          for(u32 j = 0; j < input_component_count; j++) {
            const ResourceHandle handle = task.input_ids[j];
            TaskResource& resource = _resources[handle];
            resource.ref_count--;

            if(resource.info.type == TaskResourceType::Local 
            && resource.ref_count == 0) {
              deallocations[handle] = sorted_nodes[i];
              stack.push(handle);
            }
          }
        }

        u8* buffer = (u8*)ALLOCATE
          ( sizeof(TaskNode) * task_count
          + sizeof(TaskResource) * resource_count
          );
        system->_tasks.move_ptr(buffer);
        system->_tasks.push(task_count);
        buffer += sizeof(TaskNode) * task_count;

        system->_resources.move_ptr(buffer);
        system->_resources.push(resource_count);
        for(u32 i = 0; i < task_count; i++) {
          system->_tasks[i] = _tasks[sorted_nodes[i]];
        }

        for(u32 i = 0; i < resource_count; i++) {
          system->_resources[i] = _resources[i];
        }

        return true;
      }

      b8 terminate() {
        FREE(_tasks.get_buffer());
        return true;
      }

    };
  

//    // Basic Edge class
//    template <typename NodeType, typename WeightType>
//    class WeightedEdge {
//    public:
//      NodeType destination;
//      WeightType weight;
//    
//      WeightedEdge(NodeType dest, WeightType w) : destination(dest), weight(w) {}
//    };   

    class TaskGraph {
    public:
      using TaskList = Array<TaskNode>;
      using TaskHandle = typename TaskList::index_t;

      // Basic Edge class
      struct TaskGraphEdge {
        TaskHandle destination;
      };

      using EdgeList = Array<TaskGraphEdge>;
      using AdjacencyList = SparseArray<EdgeList>;

      using IndexT = typename AdjacencyList::index_t;
      using AdjacencyMap = HashMap<TaskHandle, IndexT>;
      using IterationRange = PackedArray<IndexT>;
    private:
      TaskList _tasks;
      AdjacencyMap _adjacency_map;
      AdjacencyList _adjacencies;
      IterationRange _range;
      u64 _capacity = 0;

    public:
      TaskGraph() {}
      b8 init(const u64 capacity) {
        _capacity = capacity;
        const u64 adjacencies_size = sizeof(EdgeList) * capacity;
        const u64 adjacency_map_size = (sizeof(TaskNode) + sizeof(IndexT)) * capacity;
        const u64 iteration_range_size = sizeof(IndexT) * capacity;
        u8* allocation = (u8*)ALLOCATE
          ( adjacencies_size
          + adjacency_map_size
          + iteration_range_size
          );
        _adjacencies.move_ptr((void*)allocation);
        allocation += adjacencies_size;

        _adjacency_map.move_ptr_and_reset((void*)allocation, capacity);
        allocation += adjacency_map_size;

        _range.move_ptr((void*)allocation);
        return true;
      }

      b8 terminate() {
        RX_CHECK(_adjacencies.get_buffer() != nullptr
          , "terminated uninitialized TaskGraph");
        FREE((void*)_adjacencies.get_buffer());
        return true;
      }

      const TaskNode& operator[](const TaskHandle handle) const {
        return _tasks[handle];
      }

      const TaskNode& operator[](const TaskHandle handle) {
        return _tasks[handle];
      }

      TaskHandle add_node(TaskNode node) {
        const TaskHandle result = (TaskHandle)_tasks.get_size();
        *(_tasks.push(1)) = node;
        return result;
      }

      void add_edge(TaskHandle from, TaskHandle to) {
        if(!_adjacency_map.has(from)) {
          const auto index = _adjacencies.add_object(EdgeList{ALLOCATE(sizeof(TaskGraphEdge) * _capacity)});
          _adjacency_map.insert
            ( from
            , index
          );
          _range.add_object(index);
        }
        *(_adjacencies[_adjacency_map[from]].push(1)) = TaskGraphEdge{to};
      }

      const IndexT get_node_count() const {
        return _range.get_size();
      }
    private:
      const EdgeList& get_neighbours(TaskHandle node) const {
        return _adjacencies[_adjacency_map[node]];
      }
    
      b8 has_node(TaskHandle node) const {
        return _adjacency_map.has(node);
      }

      const EdgeList& get_adjacencies_at(const IndexT index) const { return _adjacencies[_range[index]]; 
      }

    public:
      Array<TaskGraph::TaskHandle> sort() const {

        Array<TaskGraph::TaskHandle> result;

        HashMap<TaskHandle, u32> in_degree;
        PackedArray<TaskHandle> in_degree_range;

        const auto node_count = get_node_count();

        const auto result_size = node_count * sizeof(TaskNode);
        const auto in_degree_size = node_count * sizeof(TaskNode);
        const auto in_degree_range_size = node_count * sizeof(TaskHandle);

        u8* allocation = (u8*)ALLOCATE
          ( in_degree_size
          + in_degree_range_size
          );

        result.move_ptr(ALLOCATE(result_size));

        in_degree.move_ptr_and_reset(ALLOCATE(node_count * sizeof(TaskNode)), node_count);
        allocation += in_degree_size;

        in_degree_range.move_ptr(ALLOCATE(node_count * sizeof(TaskHandle)));
        allocation += in_degree_range_size;

        for(auto i = 0; i < node_count; i++) {
          const auto& edges = get_adjacencies_at(i);
          const auto edge_count = edges.get_size();
          for(auto j = 0; j < edge_count; j++) {
            if(!in_degree.has(edges[j].destination)) {
              in_degree_range.add_object(edges[j].destination);
              in_degree.insert(edges[j].destination, 1);
              continue;
            }
            in_degree[edges[j].destination]++;
          }
          if(!in_degree.has(i)) {
            in_degree_range.add_object(i);
            in_degree.insert(i, 0);
          }
        }

        Queue<TaskGraph::TaskHandle> queue;
        const auto in_degree_count = in_degree_range.get_size();
        for (u32 i = 0; i < in_degree_count; i++) {
          const auto degree = in_degree.has(in_degree_range[i]);
          if (degree == 0) {
            queue.push(in_degree_range[i]);
          }
        }
     
        while (!(queue.get_size() == 0)) {
          TaskGraph::TaskHandle handle;
          queue.pop(&handle);
  
          // process node
          *(result.push(1)) = handle;
  
          const auto& neighbours = get_neighbours(handle);
          const auto neighbour_count = neighbours.get_size();
          for (u32 i = 0; i < neighbour_count; i++) {
            const auto edge = neighbours[i];
            if (--in_degree[edge.destination] == 0) {
                queue.push(edge.destination);
            }
          }
        }
        FREE((void*)in_degree.get_buffer());
        return result;
      }
   };
    
//    TaskGraph::TaskHandle DFS(const TaskGraph& graph, const NodeType& start, RangeType& output_range) {
//      const auto node_count = graph.get_node_count();
//      const auto visited_map_size = (sizeof(NodeType) + sizeof(b8)) * node_count;
//      const auto stack_size = sizeof(NodeType) * node_count;
//
//      u8* allocation = (u8*)ALLOCATE
//        ( visited_map_size
//        + stack_size
//        );
//      HashMap<NodeType, b8> visited{(void*)allocation};
//      allocation += visited_map_size;
//
//      Array<NodeType> stack{(void*)allocation};
//
//      *(stack.push(1)) = start;
//   
//      u32 current_stack_size = 1;
//      while (current_stack_size) {
//
//        NodeType node = stack[current_stack_size - 1];
//        stack.pop(1);
//    
//        if (visited.has(node)) continue;
//        visited.insert(node, true);
//    
//        *(output_range.push(1)) = node; // Process node
//   
//        const auto& neighbours = graph.get_neighbours(node);
//        const auto neighbour_count = neighbours.get_size();
//        for (u64 i = 0; i < neighbour_count; i++) {
//          const auto& edge = neighbours[i];
//          if (!visited.has(edge.destination)) {
//            stack.push(edge.destination);
//          }
//        }
//        current_stack_size = stack.get_size();
//      }
//      FREE((void*)allocation);
//    }
//    
//    template <typename NodeType, typename RangeType>
//    void BFS(const TaskGraph& graph, const NodeType& start, RangeType& output_range) {
//      const auto node_count = graph.get_node_count();
//      const auto visited_map_size = (sizeof(NodeType) + sizeof(b8)) * node_count;
//      const auto queue_size = sizeof(NodeType) * node_count;
//      u8* allocation = (u8*)ALLOCATE
//        ( visited_map_size
//        + queue_size
//        );
// 
//      HashMap<NodeType, b8> visited{(void*)allocation};
//      allocation += visited_map_size;
//
//      Queue<NodeType> queue{(void*)allocation, node_count};
//
//      queue.push(start);
//
//      visited.insert(start, true);
//   
//      u32 current_queue_size = 1;
//      while (current_queue_size) {
//        NodeType node;
//        queue.pop(&node);
//
//        *(output_range.push(1)) = node; // Process node
//    
//        const auto& neighbours = graph.get_neighbours(node);
//        const auto neighbour_count = neighbours.get_size();
//        for (u64 i = 0; i < neighbour_count; i++) {
//          const auto& edge = neighbours[i];
//          if (!visited.has(edge.destination)) {
//            visited.insert(edge.destination, true);
//            queue.push(edge.destination);
//          }
//        }
//        current_queue_size = queue.get_size();
//      }
//      FREE((void*)allocation);
//    }
    //#include <cmath>
    //#include <functional>
    //#include <unordered_map>
    //
    //template <typename NodeType, typename WeightType>
    //class AStar {
    //public:
    //    struct NodeInfo {
    //        WeightType gCost;
    //        WeightType fCost;
    //        NodeType parent;
    //    };
    //
    //    AStar(Graph<NodeType, WeightType>& graph, NodeType start, NodeType goal, std::function<WeightType(NodeType, NodeType)> heuristic)
    //        : graph(graph), start(start), goal(goal), heuristic(heuristic) {}
    //
    //    void search() {
    //        std::unordered_map<NodeType, NodeInfo> nodeInfo;
    //        auto compare = [](const NodeType& lhs, const NodeType& rhs) {
    //            return nodeInfo[lhs].fCost > nodeInfo[rhs].fCost;
    //        };
    //        std::priority_queue<NodeType, std::vector<NodeType>, decltype(compare)> openSet(compare);
    //
    //        nodeInfo[start] = {0, heuristic(start, goal), start};
    //        openSet.push(start);
    //
    //        while (!openSet.empty()) {
    //            NodeType current = openSet.top();
    //            openSet.pop();
    //
    //            if (current == goal) {
    //                reconstructPath(current);
    //                return;
    //            }
    //
    //            for (const auto& edge : graph.getNeighbors(current)) {
    //                NodeType neighbor = edge.destination;
    //                WeightType tentativeG = nodeInfo[current].gCost + edge.weight;
    //
    //                if (nodeInfo.find(neighbor) == nodeInfo.end() || tentativeG < nodeInfo[neighbor].gCost) {
    //                    nodeInfo[neighbor] = {tentativeG, tentativeG + heuristic(neighbor, goal), current};
    //                    openSet.push(neighbor);
    //                }
    //            }
    //        }
    //    }
    //
    //private:
    //    void reconstructPath(NodeType current) {
    //        std::vector<NodeType> path;
    //        while (current != start) {
    //            path.push_back(current);
    //            current = nodeInfo[current].parent;
    //        }
    //        path.push_back(start);
    //        std::reverse(path.begin(), path.end());
    //
    //        for (const auto& node : path) {
    //            std::cout << node << " ";
    //        }
    //    }
    //
    //    Graph<NodeType, WeightType>& graph;
    //    NodeType start, goal;
    //    std::function<WeightType(NodeType, NodeType)> heuristic;
    //};





  }		// -----  end of namespace rlsl  ----- 

}		// -----  end of namespace roxi  ----- 
