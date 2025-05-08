// =====================================================================================
//
//       Filename:  rx_dynamic_ecs.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-11-26 8:34:00 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "rx_allocator.hpp"
#include "rx_vocab.h"
#include "rx_ecs.hpp"

namespace roxi {
 
  class Table {
  private:
    struct ComponentData {
      void* data;
      u32 component_size;
    };
    SparseArray<u32> _entity_ids;
    Array<u32> _index_to_entity_ids;
    Array<ComponentData> _columns;
    u32 _max_entities;

  public:

    b8 init(const u32* component_sizes, const u32 num_components, const u32 max_num_entities);

    const u32 add_entity();

    const u32 get_size() const;

    const u32 get_num_components() const;

    void* get_component(const u32 component_index);

    void* get_component_for_entity(const u32 component_index, const u32 id);

    const u32 get_component_size(const u32 component_index);

    void remove_entity(const u32 id);

    void reset();

    b8 terminate();

  };


  class SystemData {
  private:
    using Name = const char[64];

    Name _name;

    Array<u32> _input_component_ids;
    Array<u32> _output_component_ids;

  public:
    b8 init(const char* name, const u32 num_input_components, const u32* input_component_ids, const u32 num_output_components, const u32* output_component_ids) {
      lofi::cstr_copy(name, (char*)_name, 64);

      _input_component_ids.move_ptr(ALLOCATE(sizeof(u32) * num_input_components));
      _input_component_ids.push(num_input_components);

      _output_component_ids.move_ptr(ALLOCATE(sizeof(u32) * num_output_components));
      _output_component_ids.push(num_output_components);

      const u32 iter_count = MAX(num_input_components, num_output_components);

      for(u32 i = 0; i < iter_count; i++) {
        if(i < num_input_components) {
          _input_component_ids[i] = input_component_ids[i];
        }
        if(i < num_output_components) {
          _output_component_ids[i] = output_component_ids[i];
        }
      }

      return true;
    }

    b8 update() {

      return true;
    }

    b8 terminate() {
      FREE(_input_component_ids.get_buffer());
      FREE(_output_component_ids.get_buffer());
      return true;
    }
  };

  static constexpr u32 MaxEntitiesForSmallArchetype = 16;
  static constexpr u32 MaxEntitiesForMedArchetype = 256;
  static constexpr u32 MaxEntitiesForLargeArchetype = KB(5);
  static constexpr u32 MaxArchetypeEdges = 256;

  class ECS {
  public:
    struct ComponentData {
      using Name = const char[64];

      struct MemberData {
        Name name;
        u32 size;
      };

      ComponentData() {};

      // TODO: need to finish assignment operator
      ComponentData& operator=(const ComponentData& rhs) {
        u32 i = 0;
        while(rhs._name[i] != '\0') {
          ((char*)_name)[i] = rhs._name[i++];
        }
        ((char*)_name)[i] = rhs._name[i++];
        return *this;
      }

      b8 init(const char* name, const u32 num_members, const u32* member_sizes, const char** member_names);

      MemberData& get_member(const u32 idx);

      b8 terminate();

      Name _name{};
      Array<MemberData> _members;
    };

    //struct EntityID {
    //  union {
    //    struct {
    //      u32 handle : 30;
    //      u32 generation :30;
    //      u32 type_bits : 4;
    //    } entity;

    //    struct {
    //      u32 handle : 30;
    //      u32 relationship : 30;
    //      u32 type_bits : 4;
    //    } component;
    //  };
    //};
    using EntityID = u32;
    using ComponentID = EntityID;
    using ArchetypeID = u16;

  private:

    template<typename T>
    static const ComponentID _get_component_id() {
      static const ComponentID result = _num_components++;
      return result;
    }

    ecs::RoxiStaticECS _static_ecs;

    using ColumnIndex = u16;
    struct Archetype {
      struct ArchetypeNode {
        ArchetypeID id : 15;
        b8 add : 1 = true;
      };
      Array<ComponentID> component_ids;
      LinearMap<ComponentID, ColumnIndex> column_indices;
      LinearMap<ComponentID, ArchetypeNode> archetypes_map;
    };

    struct ArchetypeRecord {
      u16 row_id = MAX_u16;
      ArchetypeID id = MAX_u16;
    };

    struct Component {
      u32 size;
    };

    struct ComponentInArchetypes {
      Array<ArchetypeID> archetypes;
    };

    SparseArray<ArchetypeRecord> _entities;
    Array<Archetype> _archetypes;
    Table* _tables;
    Component* _components;
    ComponentInArchetypes* _component_in_archetypes_map;
    LinearMap<ComponentID, ComponentData> _component_data;
    static u32 _num_components;

    inline void add_entity_to_archetype(const ComponentID component_id, const EntityID entity_id) {
      const u32 num_archetypes = _archetypes.get_size();
      for(u32 i = 0; i < num_archetypes; i++) {
        auto& component_ids = _archetypes[i].component_ids;
        const u32 num_components = component_ids.get_size();
        if((num_components == 1) && (component_id == component_ids[0])) {
          _entities[entity_id].id = i;
          _entities[entity_id].row_id = _tables[i].add_entity();
          return;
        }
      }
      const u32 id = create_new_archetype(&component_id, 1);
      _entities[entity_id].id = id;
      _entities[entity_id].row_id = _tables[id].add_entity();
    }

    inline ArchetypeID create_new_archetype(const ComponentID* const component_ids, const u32 num_components) {
      const ArchetypeID archetype_id = _archetypes.get_size();
      auto& archetype = *(_archetypes.push(1));
      Table& table = _tables[archetype_id];
      u8* archetype_allocation = (u8*)ALLOCATE(
          sizeof(ComponentID) * num_components
        + (sizeof(ComponentID) + sizeof(ColumnIndex)) * num_components
        + (sizeof(ComponentID) + sizeof(Archetype::ArchetypeNode)) * MaxArchetypeEdges);

      RX_RETURN
        ( archetype_allocation != nullptr
        , "failed to allocate archetype"
        , MAX_u16);

      archetype.component_ids.move_ptr(archetype_allocation);
      archetype_allocation += sizeof(ComponentID) * num_components;
      ComponentID* const component_ids_begin = archetype.component_ids.push(num_components);

      archetype.column_indices.move_ptr(archetype_allocation);
      archetype.column_indices.reset();
      archetype_allocation += (sizeof(ComponentID) + sizeof(ColumnIndex)) * num_components;

      archetype.archetypes_map.move_ptr(archetype_allocation);
      archetype.archetypes_map.reset();

      Array<u32> component_sizes{ALLOCATE(sizeof(u32) * num_components)};
      u32* const component_sizes_begin = component_sizes.push(num_components);
      for(u32 i = 0; i < num_components; i++) {
        const ComponentID component_id = component_ids[i];
//        *(_component_in_archetypes_map[component_ids[i]].archetypes.push(1)) = archetype_id;
        component_ids_begin[i] = component_id;
        if(_components[component_id].size == 0) {
          archetype.column_indices.insert(component_ids[i], MAX_u16);
        }
      }

      RX_RETURN
        ( table.init(component_sizes.get_buffer(), component_sizes.get_size(), MaxEntitiesForLargeArchetype)
        , "failed to init table in create new archetype"
        , MAX_u16);
      FREE(component_sizes.get_buffer());

      return archetype_id;
    }

    inline void move_entity(const u16 src_row_id, const ArchetypeID src_archetype_id, const ArchetypeID dst_archetype_id) {
      Table& src_table = _tables[src_archetype_id];
      Table& dst_table = _tables[dst_archetype_id];
      const Archetype& src_archetype = _archetypes[src_archetype_id];
      const Archetype& dst_archetype = _archetypes[dst_archetype_id];
      EntityID new_entity_id = dst_table.add_entity();
      const u32 num_src_components = src_archetype.component_ids.get_size();
      const u32 num_dst_components = dst_archetype.component_ids.get_size();

      for(u32 s = 0, d = 0; s < num_src_components && d < num_dst_components;) {
        if(src_archetype.component_ids[s] == dst_archetype.component_ids[d]) {
          const u32 component_size = src_table.get_component_size(s);
          void* src_ptr = (void*)((u8*)src_table.get_component_for_entity(s, src_row_id));
          void* dst_ptr = (void*)((u8*)dst_table.get_component_for_entity(d, new_entity_id));
          MEM_COPY(dst_ptr, src_ptr, component_size);
          s++;
        } else if(dst_archetype.component_ids[d] > src_archetype.component_ids[s]) {
          s++;
          continue;
        }
        d++;
      }
    }

    inline b8 create_new_remove_component(const ArchetypeID archetype_id, const ComponentID component_id) {

      const u32 num_src_components = _archetypes[archetype_id].component_ids.get_size();
      const u32 num_archetypes = _archetypes.get_size();

      for(u32 i = 0; i < num_archetypes; i++) {
        if(i == archetype_id)
          continue;
        b8 br = true;
        const u32 num_components = _archetypes[i].component_ids.get_size();
        if(num_components == (num_src_components - 1)) {
          for(u32 j = 0, k = 0; k < num_src_components; k++) {
            const u32 src_arch_component_id = _archetypes[archetype_id].component_ids[k];
            const u32 dst_arch_component_id = _archetypes[i].component_ids[j];
            if(component_id == src_arch_component_id) {
              continue;
            }
            if(src_arch_component_id == dst_arch_component_id) {
              j++;
              continue;
            }
            br = false;
            break;
          }
          if(br) {
            br = false;
            break;
          }
          br = true;
        } else {
          continue;
        }
        if(!br) {
          _archetypes[archetype_id].archetypes_map.insert(component_id, {(u16)i, false});
          return true;
        }
      }

      // create new archetype
      StackArray<ComponentID> component_ids{};
      auto& src_component_ids = _archetypes[archetype_id].component_ids;
      const u32 num_components = src_component_ids.get_size() - 1;
      ComponentID* const component_ids_begin = component_ids.push(num_components);
      b8 found = false;
      for(u32 i = 0, j = 0; j < num_components; i++) {
        if(component_id == src_component_ids[i]) {
          continue;
        }
        component_ids_begin[j++] = src_component_ids[i];
      }
      _archetypes[archetype_id].archetypes_map.insert(component_id, {create_new_archetype(component_ids.get_buffer(), component_ids.get_size()), false});
      return true;
    }

    inline b8 create_new_add_component(const ArchetypeID archetype_id, const ComponentID component_id) {
      // find archetype with all the src archetype's components plus the new component
      const u32 num_src_components = _archetypes[archetype_id].component_ids.get_size();
      const u32 num_archetypes = _archetypes.get_size();
      for(u32 i = 0; i < num_archetypes; i++) {
        if(i == archetype_id)
          continue;
        b8 br = true;
        const u32 num_components = _archetypes[i].component_ids.get_size();
        if(num_components == (num_src_components + 1)) {
          for(u32 j = 0, k = 0; j < num_components; j++) {
            const u32 src_arch_component_id = _archetypes[archetype_id].component_ids[k];
            const u32 dst_arch_component_id = _archetypes[i].component_ids[j];
            if(component_id == src_arch_component_id) {
              return false;
            }
            if(dst_arch_component_id == component_id) {
              continue;
            }
            if(src_arch_component_id == dst_arch_component_id) {
              continue;
            }
            br = false;
            break;
          }
          if(br) {
            br = false;
            break;
          }
          br = true;
        } else {
          continue;
        }
        if(!br) {
          _archetypes[archetype_id].archetypes_map.insert(component_id, {(u16)i, true});
          return true;
        }
      }

      // create new archetype
      StackArray<ComponentID> component_ids{};
      auto& src_component_ids = _archetypes[archetype_id].component_ids;
      const u32 num_components = src_component_ids.get_size() + 1;
      ComponentID* const component_ids_begin = component_ids.push(num_components);
      b8 found = false;
      for(u32 i = 0, j = 0; j < num_components; j++) {
        if(!found && component_id < src_component_ids[i]) {
          found = true;
          component_ids_begin[j] = component_id;
        }
        component_ids_begin[j] = src_component_ids[i++];
      }
      _archetypes[archetype_id].archetypes_map.insert(component_id, {create_new_archetype(component_ids.get_buffer(), component_ids.get_size()), true});
      return true;
    }
  public:
    b8 init(const u32 max_num_entities, const u32 max_num_components, const u16 max_num_archetypes);

    b8 add_component(const ComponentID component_id, const EntityID entity_id);

    ecs::RoxiStaticECS& get_static_ecs();

    void select(const ComponentID* const component_ids, const u32 num_component_ids, u32* archetype_count, ArchetypeID* archetypes_out);

    const u32 get_table_entity_count(const ArchetypeID archetype_id);

    void* get_table_column(const ArchetypeID archetype_id, const ComponentID component_id);

    Table& get_table(const ArchetypeID archetype_id);

    void* get_component(const ComponentID component_id, const EntityID entity_id);

    b8 remove_component(const ComponentID component_id, const EntityID entity_id);

    template<typename T>
    const ComponentID get_component_id() {
      return _get_component_id<T>();
    }

    template<typename T>
    const ComponentID create_component() {
      static const ComponentID result = _get_component_id<T>();
      _components[result] = {sizeof(T)};
      _component_in_archetypes_map[result].archetypes.move_ptr(ALLOCATE(sizeof(ArchetypeID) * 256));
      return result;
    }

    const ComponentID create_component(const char* name, const u32 num_members, const u32* const member_sizes, const char** member_names);

    const EntityID create_entity();

    b8 terminate();
  };


}		// -----  end of namespace roxi  ----- 

