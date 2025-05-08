// =====================================================================================
//
//       Filename:  ecs.hpp
//
//    Description:  entity component system header 
//
//        Version:  1.0
//        Created:  2024-06-25 2:04:01 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include <bitset>
#include <stdlib.h>
#include "..\l_database.hpp"
#include "..\l_tuple.hpp"
#include "l_component.hpp"
#include "l_entity.hpp"

namespace lofi {
  namespace ecs {

    static constexpr u64 Alignment = 8;

    template<u64 Size, typename... ComponentTs>
    struct ArchetypeDescriptor {
      using type = List<ComponentTs...>;
      static constexpr u64 size = Size;
    };

    template<class... Ts>
    using ComponentList = List<Ts...>;

    template<class... ComponentListTs>
    using ArchetypeList = List<ComponentListTs...>;

    template<class... Tags>
    using TagList = List<Tags...>;

    namespace impl {

      template<class Settings>
      struct ArchetypeComponentArrayConfig;

      template<class Settings>
      class ArchetypeComponentArrayStorage; 

    }		// -----  end of namespace impl  ----- 


    template<class CompListT, class TagListT, class ArchetypeListT>
    struct ECSDescriptor; 

    template<class... CompListTs, class... TagListTs, class... ArchetypeListTs>
    struct ECSDescriptor<List<CompListTs...>, List<TagListTs...>, List<ArchetypeListTs...>> {
      using ComponentList = List<CompListTs...>;
      using TagList = List<TagListTs...>;
      using ArchetypeList = List<ArchetypeListTs...>;
    };

    template<u64 DBID>
    struct ECSSettings {
      using type = ECSDescriptor<List<>, List<>, List<>>;
    };

    template<>
    struct ECSSettings<0>;

    template<>
    struct ECSSettings<1>;

    template<>
    struct ECSSettings<2>;

    template<>
    struct ECSSettings<3>;

    template<>
    struct ECSSettings<4>;

    template<>
    struct ECSSettings<5>;

    template<>
    struct ECSSettings<6>;

    template<>
    struct ECSSettings<7>;

    template<u64 DBID>
    using ecs_settings_t = typename ECSSettings<DBID>::type;

    template
      < class ComponentListT
      , class TagListT
      , class ArchetypeSignatureListT
      >
    struct ECSConfigImpl;

    template
      < class... ComponentTs
      , class... TagTs 
      , class... ArchetypeSignatureTs
      >
    struct ECSConfigImpl<List<ComponentTs...>, List<TagTs...>, List<ArchetypeSignatureTs...>> {
    public:
      using Components = List<ComponentTs...>;
      using Tags = List<TagTs...>;
      using ArchetypeSignatures = List<ArchetypeSignatureTs...>;
      using ThisType = ECSConfigImpl<Components, Tags, ArchetypeSignatures>;

      using ArchetypeComponentArrayConfig = impl::ArchetypeComponentArrayConfig<ThisType>;
      using ArchetypeComponentArrayStorage = impl::ArchetypeComponentArrayStorage<ThisType>;

      template<u64 Index>
      using archetype_from_index = meta::at_t<ArchetypeSignatures, Index>;

      template<u64 Index>
      using component_from_index = meta::at_t<Components, Index>;

      template<u64 Index>
      using tag_from_index = meta::at_t<Tags, Index>;

      static constexpr u64 ComponentCount() {
        return sizeof...(ComponentTs);
      }

      static constexpr u64 TagCount() {
        return sizeof...(TagTs);
      }

      static constexpr u64 ArchetypeSignatureCount() {
        return sizeof...(ArchetypeSignatureTs);
      }

      static_assert(ComponentCount() != 0, "there must be at least 1 component");
      static_assert(TagCount() != 0, "there must be at least 1 tag");
      static_assert(ArchetypeSignatureCount() != 0, "there must be at least 1 archetype");

      template<class T>
      static constexpr b8 IsComponent() {
        return lofi::has_t<T>::template type<ComponentTs...>::value;
      }

      template<class T>
      static constexpr b8 IsTag() {
        return lofi::has_t<T>::template type<TagTs...>::value;
      }

      template<class T>
      static constexpr b8 IsArchetypeSignature() {
        return lofi::has_t<T>::template type<ArchetypeSignatureTs...>::value;
      }

      template<class T>
      static constexpr u64 GetComponentIndex() {
        return lofi::meta::find_t<T>::template type<ComponentTs...>::value;
      }

      template<class T>
      static constexpr u64 GetTagIndex() {
        return lofi::meta::find_t<T>::template type<TagTs...>::value;
      }

      template<typename T>
      static constexpr u64 GetComponentTagUnionIndex() {
        const u64 ComponentIndex = GetComponentIndex<T>();
        const u64 TagIndex = GetTagIndex<T>();
        return conditional<(ComponentIndex == MAX_u64), IdxT<ComponentCount() + TagIndex>, IdxT<ComponentIndex>>::type::value;
      }

      template<class T>
      static constexpr u64 GetArchetypeSignatureIndex() {
        return lofi::meta::find_t<T>::template type<ArchetypeSignatureTs...>::value;
      }
      
      template<class T>
      static constexpr size_t ComponentBit() {
        return GetComponentIndex<T>();
      }

      template<class T>
      static constexpr size_t TagBit() {
        return ComponentCount() + GetTagIndex<T>();
      }

    };

    template<u64 DBID>
    using Config = ECSConfigImpl
      < typename ecs_settings_t<DBID>::ComponentList
      , typename ecs_settings_t<DBID>::TagList
      , typename ecs_settings_t<DBID>::ArchetypeList
      >;

    namespace impl {
      
      template<class SettingsT>
      struct ArchetypeComponentArrayConfig {

        using Settings = SettingsT;
        using ThisType = ArchetypeComponentArrayConfig<Settings>;
        using SignatureList = typename Settings::ArchetypeSignatures;
        template<typename... Ts>
        using IndexArraySeqT = List<IdxT<Settings::template GetComponentTagUnionIndex<Ts>()>...>;

        template<u64 Index>
        using type_at = meta::at_t<SignatureList, Index>;

        template<typename T>
        using ApplyIndexArraySeqT = typename meta::lift_t<IndexArraySeqT>::template type<typename T::type>::apply;

        template<typename T>
        static constexpr auto ArrayStorage = index_array<ApplyIndexArraySeqT<T>>;

        template<typename T>
        static constexpr u64 ArrayStorageSize = list_size<typename T::type>::value;

        template<u64 Index>
        static constexpr u64 ArrayStorageSizeAt = ArrayStorageSize<type_at<Index>>;

        template<class T>
        using is_component_filter = Bool<Settings::template IsComponent<T>()>;

        template<class T>
        using is_tag_filter = Bool<Settings::template IsTag<T>()>;

        template<class SignatureT>
        using SignatureComponents = typename lofi::filter<is_component_filter, SignatureT>;

        template<class SignatureT>
        using SignatureTags = typename lofi::filter<is_tag_filter, SignatureT>;
        
        template<class T>
        static constexpr u64 NumComps() {
          return list_size<SignatureComponents<T>>::value;
        }

        template<class T>
        static constexpr u64 NumTags() {
          return list_size<SignatureTags<T>>::value;
        }
 
      };

      template<class ConfigT>
      class TableDescriptorAdapter {
      private:
        using config_t = ConfigT;
        using archetypes_t = typename ConfigT::ArchetypeSignatures;

        template<typename T>
        struct table_descriptor {
          template<typename TList>
          struct type;
          template<typename... Ts>
          struct type<List<Ts...>> {
            using apply = TableDescriptor<T::size, Ts...>;
          };
          using apply = typename type<typename T::type>::apply;
        };

        template<typename TList>
        struct type;
        template<typename... Ts>
        struct type<List<Ts...>> {
          using apply = db::TableDescriptorSet<Alignment, typename table_descriptor<Ts>::apply...>;
        };

      public:
        using apply = typename type<typename config_t::ArchetypeSignatures>::apply;
      };



    }		// -----  end of namespace impl  ----- 
    
    template<class ConfigT>
    class Manager {
    private:
      using config_t = ConfigT;
      using archetype_array_config_t = typename config_t::ArchetypeComponentArrayConfig;

      using entity_pool_t = mem::SparseArrayContainerPolicy<u32, MAX_ENTITIES, 8, mem::StackAllocPolicy>;
//      using system_array_t = mem::SparseArrayContainerPolicy<System, 256, 8, mem::SubAllocPolicy>;

      static constexpr u64 NumComponents = config_t::ComponentCount();
      static constexpr u64 NumTags = config_t::TagCount();
      static constexpr u64 NumArchetypes = config_t::ArchetypeSignatureCount();

      template<typename T>
      static constexpr auto archetype_t = archetype_array_config_t::template ArrayStorage<T>;

      using table_descriptor_set_t = typename impl::TableDescriptorAdapter<config_t>::apply;
      using database_t = Database<table_descriptor_set_t>;

      template<u64 Index>
      using table_at_index = typename meta::at_t<typename database_t::tables_t::template type_at_index<Index>, Index>;

      // a metafunction to check whether a component is owned by an archetype
      // archetype_search_t<ComponentT>::template type<ArchetypeT>; returns a bool value type
      template<typename ComponentT>
      struct archetype_search_t {
        // give an archetype to the search predicate to check whether it has a given ComponentT
        static constexpr u64 ComponentIndex = config_t::template GetComponentTagUnionIndex<ComponentT>();
        template<typename ListT>
        using search_t = typename meta::lift_t<has_t<IdxT<ComponentIndex>>::template type>::template type<ListT>::apply;

        template<typename ArchetypeT>
        using type = Value<b8, search_t<typename archetype_array_config_t::template ApplyIndexArraySeqT<ArchetypeT>>::value>;
      };

      template<typename ComponentT>
      struct get_component_column_index_t {
        static constexpr u64 ComponentIndex = config_t::template GetComponentTagUnionIndex<ComponentT>();
        template<typename ArchetypeT>
        using type = typename meta::lift_t<meta::find_t<IdxT<ComponentIndex>>::template type>::template type<typename archetype_array_config_t::template ApplyIndexArraySeqT<ArchetypeT>>::apply;
      };

      template<typename ComponentT>
      struct component_search_t {
        using list_t = typename archetype_array_config_t::SignatureList;
        template<typename... Ts>
        using search_t = append_if_t<archetype_search_t<ComponentT>::template type, Ts...>;
        using search_apply = typename meta::lift_t<search_t>::template type<list_t>::apply;

        template<typename... Ts>
        using map_seq_t = List<IdxT<config_t::template GetArchetypeSignatureIndex<Ts>()>...>;

        using type = typename meta::lift_t<map_seq_t>::template type<search_apply>::apply;
      };

      // maps given component to a list of archetype indices for which archetypes own it
      template<typename ComponentT>
      using component_map_t = typename component_search_t<ComponentT>::type;

      // maps a component and archetype pair to the column index of that component inside the archetype table
      template<typename ComponentT, typename ArchetypeT>
      using component_column_map_t = typename get_component_column_index_t<ComponentT>::template type<ArchetypeT>;

    public:
      Manager() {};

      b8 init() {
        //_systems.move_ptr(RuntimeAllocator<0>::allocate(sizeof(System) * num_systems));
        //for(u64 i = 0; i < num_systems; i++) {
        //  const auto id = _systems.add_object(systems[i]);
        //}
        for(u64 i = 0; i < MAX_ENTITIES; i++) {
          _entity_generations[i] = MAX_ENTITIES;
        }
        return true;
      }

//      void move_systems(Manager& other) {
//        other._systems = _systems;
//        _systems.reset();
//        _systems.move_ptr(nullptr);
//      }

      b8 terminate() {
//        if(_systems.data() == nullptr) {
//          return false;
//        }
//        RuntimeAllocator<0>::free((void*)_systems.data());
        return true;
      }

      template<typename ArchT>
      Entity create_entity() {
       // dispatcher _dispatcher{ IdxV<NumArchetypes>
       //   , [&]<u64 I>(IdxT<I>) {
       //     return _db.template get_table<I>().insert();
       //   }};
        static constexpr u64 ArchetypeID = config_t::template GetArchetypeSignatureIndex<ArchT>();
        auto entity_id = _entities.add_object();
        _entity_ids[entity_id].archetype_id = ArchetypeID;
        _entity_ids[entity_id].row_index = _db.template get_table<ArchetypeID>().insert();
        return Entity{(u16)entity_id, (u16)_entity_generations[entity_id]};
      }

      void remove_entity(Entity entity) {
        u32 row_index = _entity_ids[entity.index].row_index;
        u32 arch_id = _entity_ids[entity.index].archetype_id;
        dispatcher _dispatcher{ IdxV<NumArchetypes>
          , [&]<u64 I>(IdxT<I>) {
            _db.template get_table<I>().remove(row_index);
          }};
        if(_entity_generations[entity.index] != entity.generation) {
          return;
        }
        _dispatcher(arch_id);
        _entity_generations[entity.index]++;
        _entities.remove_object(entity.index);
      }


      template<typename... AddTs, typename... RemoveTs>
      void add_remove_components(Entity entity, tuple<AddTs...> adds, List<RemoveTs...>) {
        dispatcher _dispatcher{ IdxV<NumArchetypes>
          , [&]<u64 SrcArchetypeIndex>(IdxT<SrcArchetypeIndex>) {
            using ArchT = typename config_t::template archetype_from_index<SrcArchetypeIndex>;
            using add_component_list = typename add_components_t<ArchT, AddTs...>::insert_component_list_t;
            using remove_component_list = typename remove_each<List<RemoveTs...>, add_component_list>::type;
            static constexpr u64 DstArchetypeIndex = find_matching_archetype_t<remove_component_list>::type::value;

          }};
      }

      template<typename... ComponentTs>
      void add_components(Entity entity) {
        using component_index_list = List<IdxT<config_t::template GetComponentTagUnionIndex<ComponentTs>()>...>;
        auto row_index = _entity_ids[entity.index].row_index;
        dispatcher _dispatcher{ IdxV<NumArchetypes>
          , [&]<u64 SrcArchetypeIndex>(IdxT<SrcArchetypeIndex>) {
            using ArchT = typename config_t::template archetype_from_index<SrcArchetypeIndex>;
            using src_arch_arg_t = typename database_t::template table_arg_t<SrcArchetypeIndex>;
            using add_component_type = add_components_t<ArchT, ComponentTs...>;
            static constexpr u64 DstArchetypeIndex = add_components_t<ArchT, ComponentTs...>::type::value;
            if constexpr (DstArchetypeIndex != MAX_u64) {
              PRINT("DstArchetype Found = %llu\n", DstArchetypeIndex);
              _entity_ids[entity.index].archetype_id = DstArchetypeIndex;
              using dst_arch_arg_t = typename database_t::template table_arg_t<DstArchetypeIndex>;
              dst_arch_arg_t dst;
              meta::static_for(
                FWD(_db.template get_table<SrcArchetypeIndex>().get_row(row_index))
                , [&]<u64 I>(IdxT<I>, typename src_arch_arg_t::template type_at_index<I>&& t) {
                  using src_arg_t = typename src_arch_arg_t::template type_at_index<I>;
                  using search_t = typename meta::lift_t<meta::find_t<src_arg_t>::template type>::template type<typename dst_arch_arg_t::type_list_t>::apply;
                  using dst_arg_t = typename dst_arch_arg_t::template type_at_index<search_t::value>;
                  dst.template get<search_t::value>() = t;
                });
                _db.template get_table<SrcArchetypeIndex>().remove(row_index);
                _entity_ids[entity.index].row_index = _db.template get_table<DstArchetypeIndex>().emplace(FWD(dst));
              }
            return;
            }};
        auto arch_id = _entity_ids[entity.index].archetype_id;
        _dispatcher(arch_id);
        arch_id = _entity_ids[entity.index].archetype_id;
      }



      template<typename... ComponentTs>
      void add_components(Entity entity, tuple<ComponentTs...>&& components) {
        using component_index_list = List<IdxT<config_t::template GetComponentTagUnionIndex<ComponentTs>()>...>;
        auto row_index = _entity_ids[entity.index].row_index;
        dispatcher _dispatcher{ IdxV<NumArchetypes>
          , [&]<u64 SrcArchetypeIndex>(IdxT<SrcArchetypeIndex>) {
            using ArchT = typename config_t::template archetype_from_index<SrcArchetypeIndex>;
            using src_arch_arg_t = typename database_t::template table_arg_t<SrcArchetypeIndex>;
            using add_component_type = add_components_t<ArchT, ComponentTs...>;
            static constexpr u64 DstArchetypeIndex = add_components_t<ArchT, ComponentTs...>::type::value;
            L_ASSERT(DstArchetypeIndex != MAX_u64 && "destination archetype does not exist in remove_components()");
            if constexpr (DstArchetypeIndex != MAX_u64) {
              _entity_ids[entity.index].archetype_id = DstArchetypeIndex;
              using dst_arch_arg_t = typename database_t::template table_arg_t<DstArchetypeIndex>;
              dst_arch_arg_t dst;

              meta::static_for(
                FWD(_db.template get_table<SrcArchetypeIndex>().get_row(row_index))
                , [&]<u64 I>(IdxT<I>, typename src_arch_arg_t::template type_at_index<I>&& t) {
                  using src_arg_t = typename src_arch_arg_t::template type_at_index<I>;
                  using search_t = typename meta::lift_t<meta::find_t<src_arg_t>::template type>::template type<typename dst_arch_arg_t::type_list_t>::apply;
                  using dst_arg_t = typename dst_arch_arg_t::template type_at_index<search_t::value>;
                  dst.template get<search_t::value>() = t;
                });
              meta::static_for(
                  static_cast<tuple<ComponentTs...>&&>(components)
                  , [&]<u64 I>(IdxT<I>, typename tuple<ComponentTs...>::template type_at_index<I>&& t) {
                    using src_arg_t = typename tuple<ComponentTs...>::template type_at_index<I>;
                    using search_t = typename meta::lift_t<meta::find_t<src_arg_t>::template type>::template type<typename dst_arch_arg_t::type_list_t>::apply;
                    using dst_arg_t = typename dst_arch_arg_t::template type_at_index<search_t::value>;
                    dst.template get<search_t::value>() = t;
                  }
                );
                _db.template get_table<SrcArchetypeIndex>().remove(row_index);
                _entity_ids[entity.index].row_index = _db.template get_table<DstArchetypeIndex>().emplace(FWD(dst));
              } else {
                PRINT_LINE("... in add_components(Entity) does not exist, please add appropriate archetype");
                L_ASSERT_BREAK();
              }
            return;
            }};
        auto arch_id = _entity_ids[entity.index].archetype_id;
        _dispatcher(arch_id);
        arch_id = _entity_ids[entity.index].archetype_id;
      }

      template<typename ComponentT>
      void remove_component(Entity entity) {
        static constexpr auto component_index = config_t::template GetComponentTagUnionIndex<ComponentT>();
        auto row_index = _entity_ids[entity.index].row_index;
        dispatcher _dispatcher{ IdxV<NumArchetypes>
          , [&]<u64 SrcArchetypeIndex>(IdxT<SrcArchetypeIndex>) {
            using ArchT = typename config_t::template archetype_from_index<SrcArchetypeIndex>;
            using src_arch_arg_t = typename database_t::template table_arg_t<SrcArchetypeIndex>;
            using remove_component_type = remove_component_t<ArchT, ComponentT>;
            static constexpr u64 DstArchetypeIndex = remove_component_t<ArchT, ComponentT>::type::value;
            if constexpr(DstArchetypeIndex != MAX_u64) {
              _entity_ids[entity.index].archetype_id = DstArchetypeIndex;
              using dst_arch_arg_t = typename database_t::template table_arg_t<DstArchetypeIndex>;
              dst_arch_arg_t dst;
              meta::static_for(
                FWD(_db.template get_table<SrcArchetypeIndex>().get_row(row_index))
                , [&]<u64 I>(IdxT<I>, typename src_arch_arg_t::template type_at_index<I>&& t) {
                  using src_arg_t = meta::at_t<typename src_arch_arg_t::type_list_t, I>;
                  using dst_arg_t = meta::at_t<typename dst_arch_arg_t::type_list_t, I>;
                  using prev_dst_arg_t = meta::at_t<typename dst_arch_arg_t::type_list_t, I - 1>;
                  if constexpr (is_same_t<src_arg_t, dst_arch_arg_t>::value) {
                    dst.template get<I>() = t;
                    return;
                  } else if constexpr (is_same_t<src_arg_t, prev_dst_arg_t>::value) {
                    dst.template get<I - 1>() = t;
                    return;
                  } else if constexpr (config_t::template IsTag<src_arg_t>()) {
                    return;
                  } else {
                    return;
                  }
                });
                _db.template get_table<SrcArchetypeIndex>().remove(row_index);
                _entity_ids[entity.index].row_index = _db.template get_table<DstArchetypeIndex>().emplace(FWD(dst));
              } 
            return;
            }};
        auto arch_id = _entity_ids[entity.index].archetype_id;
        _dispatcher(arch_id);
        arch_id = _entity_ids[entity.index].archetype_id;
      }

      template<typename... ComponentTs>
      void remove_components(Entity entity) {
        using component_index_list_t = List<IdxT<config_t::template GetComponentTagUnionIndex<ComponentTs>()>...>;
        auto row_index = _entity_ids[entity.index].row_index;
        dispatcher _dispatcher{ IdxV<NumArchetypes>
          , [&]<u64 SrcArchetypeIndex>(IdxT<SrcArchetypeIndex>) {
            using ArchT = typename config_t::template archetype_from_index<SrcArchetypeIndex>;
            using src_arch_arg_t = typename database_t::template table_arg_t<SrcArchetypeIndex>;
            using remove_component_type = remove_components_t<ArchT, ComponentTs...>;
            static constexpr u64 DstArchetypeIndex = remove_component_type::type::value;
            if constexpr(DstArchetypeIndex != MAX_u64) {
              PRINT("DstArchetype Found = %llu\n", DstArchetypeIndex);
              _entity_ids[entity.index].archetype_id = DstArchetypeIndex;
              using dst_arch_arg_t = typename database_t::template table_arg_t<DstArchetypeIndex>;
              dst_arch_arg_t dst;
              meta::static_for(
                FWD(_db.template get_table<SrcArchetypeIndex>().get_row(row_index))
                , [&]<u64 I>(IdxT<I>, typename src_arch_arg_t::template type_at_index<I>&& t) {
                  using src_arg_t = meta::at_t<typename src_arch_arg_t::type_list_t, I>;
                  using search_t = typename meta::lift_t<meta::find_t<src_arg_t>::template type>::template type<typename dst_arch_arg_t::type_list_t>::apply;
                  if constexpr (search_t::value != MAX_u64) {
                    dst.template get<search_t::value>() = t;
                    return;
                  } else {
                    return;
                  }
                });
                _db.template get_table<SrcArchetypeIndex>().remove(row_index);
                _entity_ids[entity.index].row_index = _db.template get_table<DstArchetypeIndex>().emplace(FWD(dst));
              } else {
                for_types<typename remove_component_type::remove_component_list_t> (
                    [&]<typename T>(wrapper_t<T> t) {
                    }
                  );
                PRINT_LINE("...in template remove_components(Entity) does not exist, please add appropriate archetype");
                L_ASSERT_BREAK();
              }
            return;
            }};
        auto arch_id = _entity_ids[entity.index].archetype_id;
        PRINT("arch id before remove dispatch = %llu\n", arch_id);
        _dispatcher(arch_id);
        arch_id = _entity_ids[entity.index].archetype_id;
        PRINT("arch id after remove dispatch = %llu\n", arch_id);
      }


      template<typename ArchT, typename ComponentT>
      ComponentT* get_component() {
        static constexpr u64 ComponentIndex = get_component_column_index_t<ComponentT>::template type<ArchT>::value;
        static constexpr u64 ArchetypeIndex = config_t::template GetArchetypeSignatureIndex<ArchT>();
        return _db.template get_table<ArchetypeIndex>().template get_column<ComponentIndex>();
      }

      template<typename... ComponentTs>
      struct get_archetype_indices_for_components_t {
        using component_list_t 
          = List
            < IdxT
              < config_t::template GetComponentTagUnionIndex<ComponentTs>()
              >...
            >;

        template<typename ArchetypeT>
        using pred_t = has_all_t<component_list_t, typename archetype_array_config_t::template ApplyIndexArraySeqT<ArchetypeT>>;

        template<typename... Ts>
        using filter_t = typename filter_t<pred_t>::template type<Ts...>;

        template<typename... Ts>
        using convert_to_index_t = List<IdxT<config_t::template GetArchetypeSignatureIndex<Ts>()>...>;

        using archetype_list_t = typename meta::lift_t<filter_t>::template type<typename config_t::ArchetypeSignatures>::apply;

        using type = typename meta::lift_t<convert_to_index_t>::template type<archetype_list_t>::apply;

      };
      
      template<typename... ComponentTs>
      auto find_matching_archetypes() {
        using tuple_t = tuple<ComponentTs*...>;
        using array_t = mem::ArrayContainerPolicy<tuple_t, NumArchetypes, 8, mem::SubAllocPolicy>;
        using size_array_t = mem::ArrayContainerPolicy<u32, NumArchetypes, 8, mem::SubAllocPolicy>;
        using archetype_indices_list_t = typename get_archetype_indices_for_components_t<ComponentTs...>::type;
        using return_t = tuple<array_t, size_array_t>;
        return_t result;
        result.template get<0>().move_ptr(
           StaticAllocator::allocate
              < 0
              , mem::Block<sizeof(tuple_t) * list_size<archetype_indices_list_t>::value>
              >()
          );
        result.template get<1>().move_ptr(
           StaticAllocator::allocate
              < 0
              , mem::Block<sizeof(u32) * list_size<archetype_indices_list_t>::value>
              >()
          );
        for_types<archetype_indices_list_t> (
          [&]<typename T>(wrapper_t<T>) {
            static constexpr u64 TableIndex = T::value;
            *(result.template get<1>().push(1)) = _db.template get_table<TableIndex>().get_size();
            meta::static_for(
              *(result.template get<0>().push(1))
              , [&]<u64 I>(IdxT<I>, typename tuple_t::template type_at_index<I>& t) {
                using type_t = typename meta::remove_ptr<typename tuple_t::template type_at_index<I>>::type;
                static constexpr u64 ComponentIndex 
                  = get_component_column_index_t
                    < type_t >::template type
                      < typename config_t::template archetype_from_index<TableIndex>>::value;
                t = _db.template get_table<TableIndex>().template get_column<ComponentIndex>();
              }
            );
          }
        );
        return result;
      }

      template<typename ArchT>
      u32 get_num_entities() {
        static constexpr u64 ArchetypeIndex = config_t::template GetArchetypeSignatureIndex<ArchT>();
        return _db.template get_table<ArchetypeIndex>().get_size();
      }
 
      template<typename ComponentT>
      ComponentT& get_component(Entity entity) {
        dispatcher _dispatcher{ IdxV<NumArchetypes>
          , [&]<u64 I>(IdxT<I>) {
            static constexpr u64 ComponentIndex = get_component_column_index_t<ComponentT>::template type<typename config_t::template archetype_from_index<I>>::value;
            if constexpr (ComponentIndex != MAX_u64) {
              return _db.template get_table<I>().template get_column<ComponentIndex>();
            } else {
              PRINT("entity %llu has archetype id = %llu, which does not have component idx %llu.../n", entity.index, _entity_ids[entity.index].archetype_id, config_t::template GetComponentTagUnionIndex<ComponentT>());
              L_ASSERT_BREAK();
              ComponentT dummy;
              return &dummy;
            }
          }};
        u32 arch_id = _entity_ids[entity.index].archetype_id;
        u32 row_index = _entity_ids[entity.index].row_index;
        return _dispatcher(arch_id)[row_index];
      }

      void* get_component(Entity entity, u32 component_id) {
        dispatcher _dispatcher{ IdxV<NumArchetypes>
          , [&]<u64 I>(IdxT<I>) {
            dispatcher _internal_dispatch{ IdxV<
                archetype_array_config_t::template NumComps
                < config_t::template archetype_from_index<I>
                >() 
              + archetype_array_config_t::template NumTags
                < config_t::template archetype_from_index<I>
                >()>
              , [&]<u64 J>(IdxT<J>) {
                return _db.template get_table<I>.template get_column<J>();
              }};
            return _internal_dispatch;
          }};
        u32 arch_id = _entity_ids[entity.index].archetype_id;
        u32 row_index = _entity_ids[entity.index].row_index;
        return _dispatcher(arch_id)(component_id)[row_index];
      }

      const u64 get_entity_arch_id(Entity entity) const {
        return _entity_ids[entity.index].archetype_id;
      }

      const u64 get_entity_row_index(Entity entity) const {
        return _entity_ids[entity.index].row_index;
      }

     
      void print_test_results() {
        for_types<typename config_t::ArchetypeSignatures>(
            [&]<typename T>(wrapper_t<T>) {
                static constexpr u64 SrcArchetypeIndex = config_t::template GetArchetypeSignatureIndex<T>();

                PRINT("for archetype id = %llu\n", SrcArchetypeIndex);
                for_types<typename config_t::Components>(
                [&]<typename U>(wrapper_t<U>) {
                  static constexpr u64 ComponentIndex = config_t::template GetComponentIndex<U>();
                  PRINT("\tcomponent index = %llu\n", ComponentIndex);
                  using add_component_type = add_components_t<T, U>;
                  using remove_component_type = remove_components_t<T, U>;
                  static constexpr u64 AddDstArchetypeIndex = add_components_t<T, U>::type::value;
                  static constexpr u64 RemDstArchetypeIndex = remove_components_t<T, U>::type::value;
                  PRINT("\tadd component dst archetype = %llu\n", AddDstArchetypeIndex);
                  PRINT("\tremove component dst archetype = %llu\n", RemDstArchetypeIndex);
                  using add_component_list_type = typename add_component_type::component_list_t;
                  using insert_list_type = typename add_component_type::insert_component_list_t;
                  using rem_component_list_type = typename remove_component_type::component_list_t;
                  using remove_list_type = typename remove_component_type::remove_component_list_t;
                  
                  for_types<add_component_list_type>(
                    [&]<typename V>(wrapper_t<V>) {
                      static constexpr u64 ComponentInArchetypeIndex = V::value;
                      PRINT("\t\tadd component %llu in archetype list\n", ComponentInArchetypeIndex);
                    });
                  for_types<insert_list_type>(
                    [&]<typename V>(wrapper_t<V>) {
                      static constexpr u64 InsertListIndex = V::value;
                      PRINT("\t\tcomponent %llu in archetype insert list\n", InsertListIndex);
                    });

                  for_types<rem_component_list_type>(
                    [&]<typename V>(wrapper_t<V>) {
                      static constexpr u64 ComponentInArchetypeIndex = V::value;
                      PRINT("\t\tremove component %llu in archetype list\n", ComponentInArchetypeIndex);
                    });
                  for_types<remove_list_type>(
                    [&]<typename V>(wrapper_t<V>) {
                      static constexpr u64 InsertListIndex = V::value;
                      PRINT("\t\tcomponent %llu in archetype remove list\n", InsertListIndex);
                    });
                         
                                  });
                for_types<typename config_t::Tags>(
                [&]<typename U>(wrapper_t<U>) {
                  static constexpr u64 ComponentIndex = config_t::template GetComponentTagUnionIndex<U>();
                  PRINT("\tadd component index = %llu\n", ComponentIndex);
                  using add_component_type = add_components_t<T, U>;
                  using remove_component_type = remove_components_t<T, U>;
                  static constexpr u64 AddDstArchetypeIndex = add_components_t<T, U>::type::value;
                  static constexpr u64 RemDstArchetypeIndex = remove_components_t<T, U>::type::value;
                  PRINT("\tadd dst archetype = %llu\n", AddDstArchetypeIndex);
                  PRINT("\trem dst archetype = %llu\n", RemDstArchetypeIndex);
                  using add_component_list_type = typename add_component_type::component_list_t;
                  using insert_list_type = typename add_component_type::insert_component_list_t;
                  using rem_component_list_type = typename remove_component_type::component_list_t;
                  using remove_list_type = typename remove_component_type::remove_component_list_t;
                  
                  for_types<add_component_list_type>(
                    [&]<typename V>(wrapper_t<V>) {
                      static constexpr u64 ComponentInArchetypeIndex = V::value;
                      PRINT("\t\tcomponent %llu in archetype list\n", ComponentInArchetypeIndex);
                    });
                  for_types<insert_list_type>(
                    [&]<typename V>(wrapper_t<V>) {
                      static constexpr u64 InsertListIndex = V::value;
                      PRINT("\t\tcomponent %llu in archetype insert list\n", InsertListIndex);
                    });

                  for_types<rem_component_list_type>(
                    [&]<typename V>(wrapper_t<V>) {
                      static constexpr u64 ComponentInArchetypeIndex = V::value;
                      PRINT("\t\tremove component %llu in archetype list\n", ComponentInArchetypeIndex);
                    });
                  for_types<remove_list_type>(
                    [&]<typename V>(wrapper_t<V>) {
                      static constexpr u64 InsertListIndex = V::value;
                      PRINT("\t\tcomponent %llu in archetype remove list\n", InsertListIndex);
                    });
                         
                });
                for_types<typename config_t::Components>(
                [&]<typename U>(wrapper_t<U>) {
                  static constexpr u64 ComponentIndex = config_t::template GetComponentTagUnionIndex<U>();
                  static constexpr u64 DstArchetypeIndex = remove_component_t<T, U>::type::value;
                  PRINT("for component id = %llu dst archetype = %llu\n", ComponentIndex, DstArchetypeIndex);

                });
                for_types<typename config_t::Tags>(
                [&]<typename U>(wrapper_t<U>) {
                  static constexpr u64 TagIndex = config_t::template GetComponentTagUnionIndex<U>();
                  static constexpr u64 DstArchetypeIndex = remove_component_t<T, U>::type::value;
                  PRINT("for component id = %llu dst archetype = %llu\n", TagIndex, DstArchetypeIndex);
                });
              });
      }

//
//      void add_system(System new_system) {
//        const u64 num_systems = _systems.get_size();
//        for(u64 i = 0; i < num_systems; i++) {
//          if(new_system == _systems[i]) {
//            PRINT_LINE("[WARN] trying to add the same system twice");
//            return;
//          }
//        }
//        _systems.add_object(new_system);
//      }
//
//      b8 remove_system(System system) {
//        const u64 num_systems = _systems.get_size();
//        for(u64 i = 0; i < num_systems; i++) {
//          if(system == _systems[i]) {
//            _systems.remove_object(i);
//            return true;
//          }
//        }
//        PRINT_LINE("[WARN] trying to remove system that was never added");
//        return false;
//      }
//
//      u16 get_num_systems() {
//        return _systems.get_size();
//      }
//
//      System& get_system(u16 system_index) {
//        return _systems[system_index];
//      }
//

    private:
      template<typename ArchT, typename ComponentT>
      struct remove_component_t {
        static constexpr u64 ComponentIndex = config_t::template GetComponentTagUnionIndex<ComponentT>();

        using component_list_t = typename archetype_array_config_t::template ApplyIndexArraySeqT<ArchT>;
        using archetype_indices_t = typename IdxSequence<NumArchetypes>::type;

        template<typename T>
        using get_archetype_list_t = typename archetype_array_config_t::template ApplyIndexArraySeqT<T>;

        using remove_component_list_t = typename remove<IdxT<ComponentIndex>, component_list_t>::type;

        template<typename T>
        using pred_t = is_same_t<remove_component_list_t, T>;

        template<typename... Ts>
        using find_archetype_t = typename meta::pred_find_t<pred_t>::template type<get_archetype_list_t<typename config_t::template archetype_from_index<Ts::value>>...>;

        using type = typename meta::lift_t<find_archetype_t>::template type<archetype_indices_t>::apply;
      };

      template<typename ArchT, typename ComponentT>
      static constexpr u64 remove_component() {
        using archetype_idx_t = typename remove_component_t<ArchT, ComponentT>::type;
        return archetype_idx_t::value;
      }

      template<typename... ComponentTs>
      struct find_matching_archetype_t; 

      template<typename... ComponentIdxTs>
      struct find_matching_archetype_t<List<ComponentIdxTs...>> {
        using list_t = List<ComponentIdxTs...>;

        template<typename T>
        using pred_t = is_same_t<list_t, T>;

        template<typename... Ts>
        using find_archetype_t = typename meta::pred_find_t<pred_t>::template type<typename archetype_array_config_t::template ApplyIndexArraySeqT<typename config_t::template archetype_from_index<Ts::value>>...>;
        using archetype_indices_t = typename IdxSequence<NumArchetypes>::type;
        using type = typename meta::lift_t<find_archetype_t>::template type<archetype_indices_t>::apply;
      
      };

      template<typename ArchT, typename... ComponentTs>
      struct remove_components_t {
        using component_index_list = List<IdxT<config_t::template GetComponentTagUnionIndex<ComponentTs>()>...>;

        using component_list_t = typename archetype_array_config_t::template ApplyIndexArraySeqT<ArchT>;

        using remove_component_list_t = typename remove_each<component_index_list, component_list_t>::type;

        using type = typename find_matching_archetype_t<remove_component_list_t>::type;
      };

      template<typename ArchT, typename... ComponentTs>
      static constexpr u64 remove_components() {
        using archetype_idx_t = typename remove_components_t<ArchT, ComponentTs...>::type;
        return archetype_idx_t::value;
      }

      template<typename ArchT, typename... ComponentTs>
      struct add_components_t {
        using component_index_list_t = List<IdxT<config_t::template GetComponentTagUnionIndex<ComponentTs>()>...>;
        template<typename T, typename ComponentIdxT>
        using filter_t = typename meta::less_t<ComponentIdxT>::template type<T>;
        using component_list_t = typename archetype_array_config_t::template ApplyIndexArraySeqT<ArchT>;

        using insert_component_list_t = typename insert_each_if<filter_t, component_list_t, List<>, component_index_list_t>::type;
        using type = typename find_matching_archetype_t<insert_component_list_t>::type;
      };

      template<typename ArchT, typename... ComponentTs>
      static constexpr u64 add_components() {
        using archetype_idx_t = typename add_components_t<ArchT, ComponentTs...>::type;
        return archetype_idx_t::value;
      }


      template<typename ArchT, typename ComponentT>
      struct add_component_t {
        static constexpr u64 ComponentIdx = config_t::template GetComponentTagUnionIndex<ComponentT>();
        template<typename T>
        using filter_t = typename meta::less_t<IdxT<ComponentIdx>>::template type<T>;
        using component_list_t = typename archetype_array_config_t::template ApplyIndexArraySeqT<ArchT>;
        using archetype_indices_t = typename IdxSequence<NumArchetypes>::type;

        template<typename T>
        using get_archetype_list_t = typename archetype_array_config_t::template ApplyIndexArraySeqT<T>;

        using insert_component_list_t = typename insert_if<filter_t, component_list_t, List<>, IdxT<ComponentIdx>>::type;

        template<typename T>
        using pred_t = is_same_t<insert_component_list_t, T>;

        template<typename... Ts>
        using find_archetype_t = typename meta::pred_find_t<pred_t>::template type<get_archetype_list_t<typename config_t::template archetype_from_index<Ts::value>>...>;

        using type = typename meta::lift_t<find_archetype_t>::template type<archetype_indices_t>::apply;
      };

      template<typename ArchT, typename ComponentT>
      static constexpr u64 add_component() {
        using archetype_idx_t = typename add_component_t<ArchT, ComponentT>::type;
        return archetype_idx_t::value;
      }

      template<u64 Index>
      using table_at_t = typename meta::at_t<typename database_t::tables_t::template type_at_index<Index>, Index>;

      struct record {
        u32 archetype_id = MAX_u32;
        u32 row_index = MAX_u32;
      };


      database_t _db;
      entity_pool_t _entities;
      u32 _entity_generations[MAX_ENTITIES];
      record _entity_ids[MAX_ENTITIES];
      //system_array_t _systems;
    };

    // a relation tag type is just a name only type like a tag but is not sent as a taglist
    // to the main list of components, rather the relation to the tag should be sent to the
    // components list
    template<typename RelationTagType, u64 ID>
    struct Relation {
      static constexpr u64 Index = ID;
      using relation_t = RelationTagType;
      Entity entity_id{};
    };
  }		// -----  end of namespace ecs  ----- 
}		// -----  end of namespace lofi  ----- 
