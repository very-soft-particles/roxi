// =====================================================================================
//
//       Filename:  l_variant.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-02-16 10:21:49 AM
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


// thanks to Denis Yaroshevskiy

namespace lofi {
  
  namespace meta {

    template<typename T, typename... Ts>
    struct union_t {
      union {
        T head;
        union_t<Ts...> tail;
      };
      constexpr union_t() {};

      constexpr union_t(Int<0>, auto&& ...args)
        : head(FWD(args)...) {
          //PRINT_S("constructing union at index ");
          //EVAL_PRINT_ULL(Int<0>::value);
        }

      template<size_t Index>
      constexpr union_t(Int<Index>, auto&& ...args)
        : tail(IntV<Index>, FWD(args)...) {
          //PRINT_S("constructing union at index ");
          //EVAL_PRINT_ULL(Index);
        }

     // template<size_t Index>
     // void destroy() {
     //   PRINT("destroy method called at Index %llu\n", Index);
     //   if constexpr (Index == 0) return FWD(head).~T();
     //   else return tail.template destroy<Index - 1>();
     // }

      template<size_t Index>
      constexpr auto&& get() {
        //PRINT("getting union at index = %llu\n", Index);
        //EVAL_PRINT_F(head);
        if constexpr (Index == 0) return FWD(head);
        else return tail.template get<Index - 1>();
      }
    };

    template<typename T>
    struct union_t<T> {
      union {
        T head;
      };
      constexpr union_t() {};
      constexpr union_t(Int<0>, auto&& ...args)
        : head(FWD(args)...) {}

     // template<size_t Index>
     // void destroy() {
     //   PRINT("destroy method called at Index %llu\n", Index);
     //   static_assert(Index == 0);
     //   return FWD(head).~T();
     // }

      template<size_t Index>
      constexpr auto&& get() {
        //PRINT("getting union at index = %llu\n", Index);
        //EVAL_PRINT_I(head);
        static_assert(Index == 0); 
        return FWD(head);
      }
    
    };

    template<typename UnionT>
    struct union_size;

    template<typename... Ts>
    struct union_size<union_t<Ts...>> {
      static constexpr size_t value = sizeof...(Ts);
    };

  }		// -----  end of namespace meta  ----- 

  template<typename... Ts>
  class variant {
  public:
    static constexpr size_t Size = sizeof...(Ts);
    using index_t = meta::choose_index_type_t<Size>;

    constexpr variant() {}

    template<typename T>
    constexpr variant(T&& t) {
      constexpr auto idx = meta::find_t<T>::template type<Ts...>::value;
      static_assert(idx != -1, "type does not exist in the variant type list");
      //PRINT_S("constructing variant: ");
      //EVAL_PRINT_ULL(idx);
      _data.idx = idx;
      new(static_cast<void*>(&_data.data)) T(FWD(t));
    }

    //~variant() {
    //  PRINT_S("calling variant destructor\n");
    //  dispatcher _dispatch{IdxV<Size>
    //    , [&]<size_t I>(IdxT<I>) {
    //      return FWD(_data.data.template destroy<I>());
    //    }};
    //}

    template<class Visitor>
    auto visit(Visitor&& visitor) {
      dispatcher _dispatch{IdxV<Size>
                            , [&]<size_t I>(IdxT<I>) {
                                //PRINT_S("dispatcher returned index: ");
                                //EVAL_PRINT_ULL(I);
                                return FWD(visitor)(FWD(_data.data.template get<I>()));
                            }};
      return _dispatch(_data.idx);
    }

  private:
    struct {
      meta::union_t<Ts...> data;
      index_t idx;
    } _data;
  };


}		// -----  end of namespace lofi  ----- 
