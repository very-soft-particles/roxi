// =====================================================================================
//
//       Filename:  l_tuple.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-02-09 4:08:50 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "l_vocab.hpp"

// thank you to Odin Holmes and Chiel Douwes


namespace lofi {

  template<typename I, typename T>
  struct tuple_element {
    static constexpr u64 index = I::value;
    using type = T;
    T elem;
  };

 
  template<typename... Ts>
  struct tuple_base : Ts... {
    using type = List<Ts...>;
  };

//  template<typename T, typename... Ts>
//  struct tuple_zip<Int<0>, T, Ts...> : tuple_zip<Int<1>, Ts>... {
//    using type = List<tuple_element<0, T>, typename tuple_zip<Int<1>, Ts...>::type>; 
//  };

  template<typename... Ts>
  using tuple_zip = typename zipper<tuple_element, typename IdxSequence<sizeof...(Ts)>::type, Ts...>::type;

  template<typename... Ts>
  using make_tuple = typename meta::lift<tuple_base, tuple_zip<Ts...>>::type;

  template<typename... Ts>
  struct tuple : make_tuple<Ts...> {
    using base_t = make_tuple<Ts...>;
    using list_t = typename base_t::type;

    using type_list_t = List<Ts...>;
    template<u64 Index>
    using type_at_index = meta::at_t<type_list_t, Index>;

    template<u64 Index>
    using cast_type_at_index = meta::at_t<list_t, Index>;


    template<u64 Index>
    auto& get() {
      return static_cast<meta::at_t<list_t, Index>&>(*this).elem;
    }

    static constexpr u64 get_size() {
      return sizeof...(Ts);
    }
  };

  namespace meta {
    template<u64 Index, typename... Ts>
    auto& get(tuple<Ts...>& t) {
      return static_cast<meta::at_t<typename tuple<Ts...>::type, Index>&>(t);
    };

    template<u64 Index, typename... Ts>
    auto& get(tuple<Ts...>&& t) {
      return static_cast<meta::at_t<typename tuple<Ts...>::type, Index>&>(t);
    };

    template <class Tuple, class Func, u64 ...Is>
    constexpr void static_for_impl(Tuple& t, Func&& f, wrapper_t<List<IdxT<Is>...>>)
    {
        ( ..., f(IdxV<Is>, get<Is>(t).elem));
    }
    
    template <class... Ts, class Func >
    constexpr void static_for(tuple<Ts...>& t, Func &&f)
    {
        static_for_impl(t, FWD(f), wrapper_t<typename IdxSequence<sizeof...(Ts)>::type>{});
    }

    template <class Tuple, class Func, u64 ...Is>
    constexpr void static_for_impl(Tuple&& t, Func&& f, wrapper_t<List<IdxT<Is>...>>)
    {
        ( ..., f(IdxV<Is>, FWD(get<Is>(FWD(t)).elem)));
    }
    
    template <class... Ts, class Func >
    constexpr void static_for(tuple<Ts...>&& t, Func &&f)
    {
        static_for_impl(FWD(t), FWD(f), wrapper_t<typename IdxSequence<sizeof...(Ts)>::type>{});
    }


//    template <class TupleT, class TupleU, class Func, u64 ...Is>
//    constexpr void static_map_impl(TupleT& t, TupleU& u, Func& f, IdxSequence<Is...> )
//    {
//        ( ..., f(IdxV<Is>, lofi::meta::get<Is>(FWD(t))));
//    }
//    
//    template <class... Ts, class... Us, class Func>
//    constexpr void static_map(tuple<Ts...>& t, tuple<Us...>& us, Func &&f)
//    {
//        static_for_impl(t, FWD(f), IdxSequence<sizeof...(Ts)>{});
//    }
 
  }		// -----  end of namespace meta  ----- 
}		// -----  end of namespace lofi  ----- 
