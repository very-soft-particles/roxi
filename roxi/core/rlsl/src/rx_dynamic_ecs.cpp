// =====================================================================================
//
//       Filename:  rx_dynamic_ecs.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2025-01-31 12:43:17 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Robyn Gray (rg), ...
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "../include/rx_dynamic_ecs.hpp"


namespace roxi {

  b8 Table::init(const u32* component_sizes, const u32 num_components, const u32 max_num_entities) {
    _max_entities = max_num_entities;

    const u32 single_allocation_size = (sizeof(u32) * max_num_entities) + (sizeof(u32) * max_num_entities) + (sizeof(ComponentData) * num_components);
    u8* single_allocation = (u8*)ALLOCATE(single_allocation_size);

    RX_CHECK(single_allocation != nullptr
      , "failed to allocate table data");

    _entity_ids.move_ptr((void*)single_allocation);

    single_allocation += sizeof(u32) * max_num_entities;
    _index_to_entity_ids.move_ptr((void*)single_allocation);

    single_allocation += sizeof(u32) * num_components;
    _columns.move_ptr((void*)single_allocation);

    u32 allocation_size = 0;

    for(u32 i = 0; i < num_components; i++) {
      const u32 component_size = component_sizes[i];
      if(component_size == 0) {
        continue;
      }
      allocation_size += component_size;
      (_columns.push(1))->component_size = component_size;
    }

    allocation_size *= max_num_entities;
    u8* data = (u8*)ALLOCATE(allocation_size);

    RX_CHECK(data != nullptr
      , "failed to allocate columns");

    for(u32 i = 0; i < num_components; i++) {
      const u32 component_size = component_sizes[i];
      if(component_size == 0) {
        continue;
      }
      _columns[i].data = (void*)data;
      const u32 column_size = component_sizes[i] * max_num_entities;
      data += column_size;
    }

    return true;
  }

  const u32 Table::add_entity() {
    const u32 index = _index_to_entity_ids.get_size();
    const u32 id = _entity_ids.add_object(index);
    *(_index_to_entity_ids.push(1)) = id;
    return id;
  }
  
  const u32 Table::get_size() const {
    return _index_to_entity_ids.get_size();
  }

  const u32 Table::get_num_components() const {
    return _columns.get_size();
  }

  void* Table::get_component(const u32 component_index) {
    return _columns[component_index].data;
  }

  void* Table::get_component_for_entity(const u32 component_index, const u32 id) {
    return (void*)((u8*)get_component(component_index) + (get_component_size(component_index) * _entity_ids[id]));
  }

  const u32 Table::get_component_size(const u32 component_index) {
    return _columns[component_index].component_size;
  }

  void Table::remove_entity(const u32 id) {
    const u32 index = _entity_ids[id];
    const u32 num_components = _columns.get_size();
    const u32 final_entity_index = _index_to_entity_ids.get_size() - 1;
    for(u32 i = 0; i < num_components; i++) {
      u8* const component_begin = (u8*)_columns[i].data;
      const u32 component_size = _columns[i].component_size;
      void* dst_entity_data = (void*)(component_begin + (component_size * index));
      void* src_entity_data = (void*)(component_begin + (component_size * final_entity_index));
      MEM_COPY(dst_entity_data, src_entity_data, component_size);
    }
    const u32 final_entity_id = _index_to_entity_ids[final_entity_index];
    _index_to_entity_ids[index] = final_entity_id;
    _index_to_entity_ids.pop(1);
    _entity_ids[final_entity_id] = index;
  }

  void Table::reset() {
    _entity_ids.reset();
    _index_to_entity_ids.clear();
  }

  b8 Table::terminate() {
    // MUST go first because entity ids allocation contains the pointer for the columns
    FREE(_columns.get_buffer());
    FREE(_entity_ids.get_buffer());
    return true;
  }

  b8 ECS::ComponentData::init(const char* name, const u32 num_members, const u32* member_sizes, const char** member_names) {
    _members.move_ptr(ALLOCATE(sizeof(MemberData) * num_members));
    if(_members.get_buffer() == nullptr) {
      return false;
    }
    _members.push(num_members);

    lofi::cstr_copy(name, (char*)_name, 64);

    for(u32 i = 0; i < num_members; i++) {
      lofi::cstr_copy(member_names[i], (char*)_members[i].name, 64);
      _members[i].size = member_sizes[i];
    }

    return true;
  }

  ECS::ComponentData::MemberData& ECS::ComponentData::get_member(const u32 idx) {
    return _members[idx];
  }

  b8 ECS::ComponentData::terminate() {
    if(_members.get_buffer() == nullptr) {
      return false;
    }
    FREE(_members.get_buffer());
    return true;
  }

  b8 ECS::init(const u32 max_num_entities, const u32 max_num_components, const u16 max_num_archetypes) {
    u8* single_allocation = (u8*)ALLOCATE(
        sizeof(ArchetypeRecord) * max_num_entities
      + sizeof(Archetype) * max_num_archetypes
      + sizeof(Table) * max_num_archetypes
      + sizeof(Component) * max_num_components
      + sizeof(ComponentInArchetypes) * max_num_components
      + sizeof(ComponentData) * max_num_components
      );

    RX_CHECK(single_allocation != nullptr
      , "failed to allocate dynamic ecs");

    _entities.move_ptr(single_allocation);
    single_allocation += sizeof(ArchetypeRecord) * max_num_entities;

    _archetypes.move_ptr(single_allocation);
    single_allocation += sizeof(Archetype) * max_num_archetypes;

    _tables = (Table*)single_allocation;
    single_allocation += sizeof(Table) * max_num_archetypes;

    _components = (Component*)single_allocation;
    single_allocation += sizeof(Component) * max_num_components;

    _component_in_archetypes_map = (ComponentInArchetypes*)single_allocation;
    single_allocation += sizeof(ComponentInArchetypes) * max_num_components;

    _component_data.move_ptr(single_allocation);
    single_allocation += sizeof(ComponentData) + max_num_components;

    return true;
  }

  b8 ECS::add_component(const ComponentID component_id, const EntityID entity_id) {
    const ArchetypeRecord archetype_record = _entities[entity_id];
    const ArchetypeID src_id = archetype_record.id;
    if(src_id == MAX_u16) {
      add_entity_to_archetype(component_id, entity_id);
      return true;
    }
    auto& archetype_map = _archetypes[src_id].archetypes_map;
    if(!archetype_map.has(component_id)) {
      if(!create_new_add_component(src_id, component_id)) {
        return false;
      }
    }
 
    Archetype::ArchetypeNode node = archetype_map[component_id]; 
    if(!node.add) {
      return false;
    }
    const ArchetypeID dst_id = node.id;
    move_entity(archetype_record.row_id, src_id, dst_id);
    return true;
  }

  ecs::RoxiStaticECS& ECS::get_static_ecs() {
    return _static_ecs;
  }

  // working on it
  void ECS::select(const ComponentID* const component_ids, const u32 num_component_ids, u32* archetype_count, ArchetypeID* archetypes_out) {
    const u32 count = *archetype_count;
    // backtrack
    if(count == 0) {
      const auto& archetypes = _component_in_archetypes_map[component_ids[0]].archetypes;
      const u32 num_archetypes_for_component = archetypes.get_size();
      for(u32 i = 0; i < num_archetypes_for_component; i++) {
        const ArchetypeID archetype_id = archetypes[i];
        for(u32 j = 1; j < num_component_ids; j++) {
          const auto& archetypes_to_check = _component_in_archetypes_map[component_ids[j]].archetypes;
          const u32 num_archetypes_to_check = archetypes_to_check.get_size();
          b8 found = false;
          for(u32 k = 0; k < num_archetypes_to_check; k++) {
            const ArchetypeID archetype_id_to_check = archetypes_to_check[k];
            if(archetype_id != archetype_id_to_check) {
              continue;
            }
            found = true;
            break;
          }
          if(found) {
            (*archetype_count)++;
            continue;
          }
          break;
        }
      }
      return;
    }
    u32 i = 0;
    while(i != count) {
      u32 check = i;
      const auto& archetypes = _component_in_archetypes_map[component_ids[i]].archetypes;
      const u32 num_archetypes_for_component = archetypes.get_size();
      for(u32 j = 0; j < num_archetypes_for_component; j++) {
        const ArchetypeID archetype_id = archetypes[j];
        for(u32 k = 1; k < num_component_ids; k++) {
          const auto& archetypes_to_check = _component_in_archetypes_map[component_ids[k]].archetypes;
          const u32 num_archetypes_to_check = archetypes_to_check.get_size();
          b8 found = false;
          for(u32 l = 0; l < num_archetypes_to_check; l++) {
            const ArchetypeID archetype_id_to_check = archetypes_to_check[l];
            if(archetype_id != archetype_id_to_check) {
              continue;
            }
            found = true;
            break;
          }
          if(found) {
            archetypes_out[i++] = archetype_id;
            continue;
          }
        }
      }
      L_ASSERT(check + 1 == i && "incorrect archetype count supplied to select function");
    }
    return;
  }

  const u32 ECS::get_table_entity_count(const ArchetypeID archetype_id) {
    return _tables[archetype_id].get_size();
  }

  void* ECS::get_table_column(const ArchetypeID archetype_id, const ComponentID component_id) {
    const u32 column_index = _archetypes[archetype_id].column_indices[component_id];
    return _tables[archetype_id].get_component(column_index);
  }

  Table& ECS::get_table(const ArchetypeID archetype_id) {
    return _tables[archetype_id];
  }

  void* ECS::get_component(const ComponentID component_id, const EntityID entity_id) {
    const ArchetypeRecord archetype_record = _entities[entity_id];
    auto& component_ids = _archetypes[archetype_record.id].column_indices;
    if(!component_ids.has(component_id)) {
      return nullptr;
    }
    Table& table = _tables[archetype_record.id];
    const u32 component_index = component_ids[component_id];
    u8* const column_begin = (u8*)table.get_component(component_index);
    return (void*)(column_begin + (table.get_component_size(component_index) * archetype_record.row_id));
  }

  b8 ECS::remove_component(const ComponentID component_id, const EntityID entity_id) {
    const ArchetypeRecord archetype_record = _entities[entity_id];
    const ArchetypeID src_id = archetype_record.id;
    auto& archetype_map = _archetypes[src_id].archetypes_map;
    if(!archetype_map.has(component_id)) {
      if(!create_new_remove_component(src_id, component_id)) {
        return false;
      }
    }
    Archetype::ArchetypeNode node = archetype_map[component_id]; 
    if(node.add) {
      return false;
    }
    const ArchetypeID dst_id = archetype_map[component_id].id;
    move_entity(archetype_record.row_id, src_id, dst_id);
    return true;
  }

  const ECS::ComponentID ECS::create_component(const char* name, const u32 num_members, const u32* const member_sizes, const char** member_names) {
    static const ComponentID result = _num_components++;
    ComponentData data;
    data.init(name, num_members, member_sizes, member_names);
    u32 component_size = 0;
    for(u32 i = 0; i < num_members; i++) {
      String member_name = lofi::nt_str_cstring(member_names[i]);
      const u32 member_size = member_sizes[i];
      ComponentData::MemberData& member = data.get_member(i);
      lofi::str_copy(member_name, (char*)member.name, 64);
      member.size = member_size;
      component_size += member_size;
    }
    _component_data.insert(result, data);
    _components[result] = {component_size};
    _component_in_archetypes_map[result].archetypes.move_ptr(ALLOCATE(sizeof(ArchetypeID) * 256));

    return result;
  }

  const ECS::EntityID ECS::create_entity() {
    return _entities.add_object();
  }

  b8 ECS::terminate() {
    for(u32 i = 0; i < _num_components; i++) {
      _component_data[i].terminate();
      FREE(_component_in_archetypes_map[i].archetypes.get_buffer());
    }
    const u32 num_archetypes = _archetypes.get_size();
    for(u32 i = 0; i < num_archetypes; i++) {
      RX_CHECK(_tables[i].terminate()
        , "table failed to properly terminate");
      FREE(_archetypes[i].component_ids.get_buffer());
    }
    FREE(_entities.get_buffer());
    return true;
  }

  u32 ECS::_num_components = 0;   // NOLINT

}		// -----  end of namespace roxi  ----- 
