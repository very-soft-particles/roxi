// =====================================================================================
//
//       Filename:  l_database.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-02-05 7:26:08 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "l_container.hpp"

namespace lofi {

  namespace tbl {

    template<size_t Size>
    using SparseT = mem::SparseArrayContainerPolicy<uint32_t, Size, Size, mem::StackAllocPolicy>;

    template<size_t Size>
    using DenseT = mem::PackedArrayContainerPolicy<uint32_t, Size, Size, mem::StackAllocPolicy>;

    template<size_t DBID, size_t TableID, size_t ColumnID, class T>
    struct ColumnDescriptor {
      using type = T;
    };

    template<size_t DBID, size_t TableID>
    struct ColumnDescriptorSet {
      using type = List<>;
    };

    template<size_t DBID, size_t TableID>
    using column_descriptors_t = typename ColumnDescriptorSet<DBID, TableID>::type;

    template<size_t DBID, size_t TableID>
    struct get_column_size : Value<size_t, ColumnDescriptorSet<DBID, TableID>::size> {};


    // for instantiating specific tables outside a database ??
  }		// -----  end of namespace tbl  ----- 

  template<size_t TableSize, class... Ts>
  struct TableDescriptor {
    using type = List<Ts...>;
    static constexpr size_t size = TableSize;
  };

  template<class TableDescriptor, u64 Alignment>
  class Table;

  template<u64 Alignment, u64 Size, typename... Ts>
  class Table<TableDescriptor<Size, Ts...>, Alignment> {
  public:
    using list_t = List<Ts...>;
    static constexpr size_t num_fields = sizeof...(Ts);
    static constexpr size_t size = Size;

    using arg_t = typename meta::lift<tuple, list_t>::type;
    using table_t = Table<TableDescriptor<Size, List<Ts...>>, Alignment>;
    using columns_t = mem::PackedMultiArrayContainerPolicy<list_t, Size, Alignment, mem::StackAllocPolicy>;
    using sparse_t = typename tbl::SparseT<size>;
    using dense_t = typename tbl::DenseT<size>;
    using index_t = u32;

    Table() = default;

    void* operator[](index_t outer_index) {
      return columns[outer_index];
    }

    index_t emplace(arg_t&& arg) {
      auto sparse_handle = sparse.add_object();
      PRINT("sparse handle = %llu\n", (u64)sparse_handle);
      sparse[sparse_handle] = dense.add_object();
      dense[sparse[sparse_handle]] = sparse_handle;
      columns.add_object(FWD(arg));
      return sparse_handle;
    }

    arg_t get_row(index_t index) {
      arg_t result;
      auto& cols = *columns.get_multi_array();
      meta::static_for(
            cols
          , [&]<u64 I>(IdxT<I>, typename columns_t::template type_at<I>* t) {
            result.template get<I>() = t[index];
          }
        );
      return result;
    }

    index_t insert() {
      auto sparse_handle = sparse.add_object();
      PRINT("sparse handle = %llu\n", (u64)sparse_handle);
      sparse[sparse_handle] = dense.add_object();
      dense[sparse[sparse_handle]] = sparse_handle;
      columns.add_object();
      return sparse_handle;
    }

    void remove(index_t index) {
      dense.remove_object(sparse[index]);
      sparse.remove_object(index);
      columns.remove_object(index);
    }

    template<size_t Index>
    meta::at_t<list_t, Index>* get_column() {
      return columns.template get<Index>();
    }

    template<typename T>
    static constexpr size_t get_column_index() {
      using search_t = typename meta::lift<meta::find_t<T>::template type, list_t>::type;
      return search_t::value;
    }

    template<typename T>
    static constexpr b8 contains() {
      constexpr size_t value = get_column_index<T>();
      if constexpr (value != MAX_u64) {
        return true;
      }
      return false;
    }

    template<u64 OuterIndex>
    meta::at_t<list_t, OuterIndex>& at(index_t inner_index) {
      return columns.template at<OuterIndex>(inner_index);
    }

    index_t get_size() {
      return columns.get_size();
    }

  private:
    sparse_t sparse{};
    dense_t dense{};
    columns_t columns{};
  };

  namespace db {

    template<u64 Align, typename... TableDescriptorTs>
    struct TableDescriptorSet {
      using type = List<TableDescriptorTs...>;
      static constexpr u64 Alignment = Align;
    };

    template<size_t DBID>
    struct info {
      using table_descriptors_t = typename TableDescriptorSet<DBID>::type;
      static constexpr size_t table_count = list_size<table_descriptors_t>::value;

      template<size_t TableID>
      static constexpr size_t table_size = meta::at_t<table_descriptors_t, TableID>::size;

      template<typename Idx>
      using column_type = typename meta::at_t<table_descriptors_t, Idx::value>::value_t;

      template<typename T>
      struct contains_seq { 
        template<typename... Idxs>
        using type = typename meta::lift<meta::find_t<T>::template type, table_descriptors_t>::type;
      };
    };
    template<class TableDescriptorListT, u64 Alignment>
    struct tables;

    template<u64 Alignment, class... Ts>
    struct tables<List<Ts...>, Alignment> {
      using type = tuple<Table<Ts, Alignment>...>;
    };

    template<class Descriptor, u64 Alignment>
    using tables_t = typename tables<typename Descriptor::type, Alignment>::type;
  }

  template<class TableDescriptorSetT>
  class Database {
  public:
    using table_descriptors_t = TableDescriptorSetT;
    using list_t = typename table_descriptors_t::type;
    static constexpr u64 Alignment = table_descriptors_t::Alignment;
    using tables_t = db::tables_t
            <table_descriptors_t, Alignment>;

    template<u64 Index>
    using table_arg_t = typename tables_t::template type_at_index<Index>::arg_t;
    
    Database() = default;
    
    template<u64 TableID>
    constexpr auto& get_table() {
      return tables.template get<TableID>();
    }
 
  private:
    tables_t tables{};
  };

  template<class DB, u64 TableID, class Predicate>
  class Query {
  private:
    DB* _db = nullptr;
    Predicate _pred;
  public:
    Query(DB* db, Predicate&& pred) : _db{db}, _pred{pred} {}
    
    b8 matches(u64 index) {
      return _pred.matches(_db->template get_table<TableID>(), index);
    }

  };

}		// -----  end of namespace lofi  ----- 
