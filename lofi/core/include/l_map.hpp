// =====================================================================================
//
//       Filename:  l_map.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-07-16 4:19:13 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "l_vocab.hpp"
#include "l_tuple.hpp"

namespace lofi {
  namespace meta {

    template<typename T, typename U>
    struct map_element {
      using key_t = T;
      using value_t = U;
      static constexpr auto elem = U::value;
    };

    template<typename... Ts>
    struct map_base : Ts... {
      using list_t = List<Ts...>;
    };

    template<typename TList>
    struct map_zip_t;

    template<typename... Ts>
    struct map_zip_t<List<Ts...>> {
      template<typename... Us>
      using type = zipper<map_element, List<Ts...>, Us...>;
    };



    template<typename TList, typename... Us>
    using make_map 
      = typename meta::lift_t<map_base>
        ::template type<typename map_zip_t<TList>
          ::template type<Us...>::type>
            ::apply;

    template<typename KeyList, typename... Us>
    struct map : make_map<KeyList, Us...> {
      static_assert(list_size<KeyList>::value == sizeof...(Us), "unequal number of keys and values in meta::map");
      using base_t = make_map<KeyList, Us...>;
      using list_t = typename base_t::list_t;

      using key_list_t = KeyList;

      template<typename KeyType>
      using search_t = typename meta::lift_t<find_t<KeyType>::template type>::template type<key_list_t>::apply;
        
      template<typename KeyType>
      static constexpr b8 has() {
        if constexpr(search_t<KeyType>::value == MAX_u64) {
          return false;
        }
        return true;
      }

      template<typename KeyType>
      auto& get() {
        return static_cast<meta::at_t<list_t, search_t<KeyType>::value>&>(*this).elem;
      }

      static constexpr u64 get_size() {
        return sizeof...(Us);
      }
    };

    template<typename TList>
    struct map_t;

    template<typename... Ts>
    struct map_t<List<Ts...>> {
      template<typename... Us>
      using type = map<List<Ts...>, Us...>;
    };

  }		// -----  end of namespace meta  ----- 
}		// -----  end of namespace lofi  ----- 
