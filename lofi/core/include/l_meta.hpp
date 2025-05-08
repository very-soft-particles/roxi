// =====================================================================================
//
//       Filename:  l_meta.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-02-03 9:57:11 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "l_base.hpp"

namespace lofi {

  template<b8 Condition, class TrueCase, class FalseCase>
  struct conditional {
    using type = FalseCase;
  };

  template<class TrueCase, class FalseCase>
  struct conditional<true, TrueCase, FalseCase> {
    using type = TrueCase;
  };

  template<class T, class... Ts>
  struct has_type : Bool<false> {};

  template<class T, class U, class... Ts>
  struct has_type<T, U, Ts...> : has_type<T, Ts...> {};

  template<class T, class... Ts>
  struct has_type<T, T, Ts...> : Bool<true> {};

  template<class T>
  struct has_t {
    template<typename... Ts>
    using type = has_type<T, Ts...>;
  };

  template<class... Ts>
  struct List {
    template<typename T>
    using has = typename has_t<T>::template type<Ts...>;
  };

  // lists must both be sorted
  template<class TList, class UList>
  struct has_all;

  template<typename T, typename U, typename... Ts, typename... Us>
  struct has_all<List<T, Ts...>, List<U, Us...>> : has_all<List<T, Ts...>, List<Us...>> {};

  template<typename T, typename... Ts, typename... Us>
  struct has_all<List<T, Ts...>, List<T, Us...>> : has_all<List<Ts...>, List<Us...>> {};

  template<typename... Us>
  struct has_all<List<>, List<Us...>> : Return<Bool<true>> {};

  template<typename... Ts>
  struct has_all<List<Ts...>, List<>> : Return<Bool<false>> {};

  template<>
  struct has_all<List<>, List<>> : Return<Bool<true>> {};

  template<typename TList, typename UList>
  using has_all_t = typename has_all<TList, UList>::type;

  namespace meta {

    template<typename T>
    struct remove_ptr {
      using type = T;
    };

    template<typename T>
    struct remove_ptr<T*> : remove_ptr<T> {};

    struct identity {
      template<typename T>
      using f = T;
    };
  
    struct make_list {
      template<typename... Ts>
      using f = List<Ts...>;
    };

    static constexpr u64 index_switch_sizes[] = {256, 65536, 4294967296};
    static constexpr u64 index_max_sizes[] = {MAX_u8, MAX_u16, MAX_u32, MAX_u64};

    template<size_t Size>
    static constexpr u8 get_type_table_index 
            = Size <= index_switch_sizes[0] ? 
                0 : 
                Size <= index_switch_sizes[1] ? 
                  1 :
                  Size <= index_switch_sizes[2] ?
                    2 : 3;

    template<u8 index>
    struct index_type_table;

    template<>
    struct index_type_table<0> {
      using type = u8;
    };

    template<>
    struct index_type_table<1> {
      using type = u16;
    };

    template<>
    struct index_type_table<2> {
      using type = u32;
    };

    template<>
    struct index_type_table<3> {
      using type = u64;
    };

    template<size_t Size>
    using choose_index_type_t = typename meta::index_type_table<meta::get_type_table_index<Size>>::type;


  } // end of namespace meta

  template<template<class> class Filter, class TupleIn, class TupleOut = List<>>
  struct filter;

  template<template<class> class Filter, class TypeIn, class... TypesIn, class... TypesOut>
  struct filter<Filter, List<TypeIn, TypesIn...>, List<TypesOut...>> {
    using type 
      = typename conditional
      < Filter<TypeIn>::value
      , typename filter
        < Filter
        , List<TypesIn...>
        , List<TypeIn, TypesOut...>
        >::type
      , typename filter
        < Filter
        , List<TypesIn...>
        , List<TypesOut...>
        >::type
      >::type;
  };

  template<template<class> class Filter, class TypeIn, class... TypesIn>
  struct filter<Filter, List<TypeIn, TypesIn...>, List<>> {
    using type 
      = typename conditional
      < Filter<TypeIn>::value
      , typename filter
        < Filter
        , List<TypesIn...>
        , List<TypeIn>
        >::type
      , typename filter
        < Filter
        , List<TypesIn...>
        , List<>
        >::type
      >::type;
  };

  template<template<class> class Filter, class... TypesOut>
  struct filter<Filter, List<>, List<TypesOut...>> {
    using type = List<TypesOut...>;
  };

  template<template<class> class Filter>
  struct filter<Filter, List<>, List<>> {
    using type = List<>;
  };

  template<template<class> class FilterT>
  struct filter_t {
    template<typename... Ts>
    using type = typename filter<FilterT, List<Ts...>>::type;
  };

  template<class Tuple>
  struct strip_first;

  template<class Type, class... Types>
  struct strip_first<List<Type, Types...>> {
    using type = Type;
    using cdr = List<Types...>;
  };

  template<class Type>
  struct strip_first<List<Type>> {
    using type = Type;
  };

  template<class T>
  struct wrapper_t {
    using type = T;
  };

}		// -----  end of namespace lofi  ----- 
