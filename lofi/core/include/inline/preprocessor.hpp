// =====================================================================================
//
//       Filename:  preprocessor.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-02-14 5:53:34 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "../l_base.hpp"
#include "indices.inl"

#define EMPTY()
#define DEFER(...) __VA_ARGS__ EMPTY()
#define DEFER2(...) __VA_ARGS__ DEFER(EMPTY) ()
#define DEFER3(...) __VA_ARGS__ DEFER2(EMPTY) ()
#define DEFER4(...) __VA_ARGS__ DEFER3(EMPTY) ()
#define DEFER5(...) __VA_ARGS__ DEFER4(EMPTY) ()
#define DEFER6(...) __VA_ARGS__ DEFER5(EMPTY) ()
#define DEFER7(...) __VA_ARGS__ DEFER6(EMPTY) ()
#define DEFER8(...) __VA_ARGS__ DEFER7(EMPTY) ()
#define DEFER9(...) __VA_ARGS__ DEFER8(EMPTY) ()
#define DEFER10(...) __VA_ARGS__ DEFER9(EMPTY) ()
#define DEFER11(...) __VA_ARGS__ DEFER10(EMPTY) ()
#define DEFER12(...) __VA_ARGS__ DEFER11(EMPTY) ()
#define DEFER13(...) __VA_ARGS__ DEFER12(EMPTY) ()
#define DEFER14(...) __VA_ARGS__ DEFER13(EMPTY) ()
#define DEFER15(...) __VA_ARGS__ DEFER14(EMPTY) ()

#define EVAL_1(...) __VA_ARGS__
#define EVAL_2(...) EVAL_1(EVAL_1(__VA_ARGS__))
#define EVAL_3(...) EVAL_2(EVAL_2(__VA_ARGS__))
#define EVAL_4(...) EVAL_3(EVAL_3(__VA_ARGS__))
#define EVAL_5(...) EVAL_4(EVAL_4(__VA_ARGS__))
#define EVAL_6(...) EVAL_5(EVAL_5(__VA_ARGS__))
#define EVAL_7(...) EVAL_6(EVAL_6(__VA_ARGS__))
#define EVAL_8(...) EVAL_7(EVAL_7(__VA_ARGS__))

#define EVAL(...) EVAL_8(__VA_ARGS__)

#define EAT(...) 
#define RECURSE(x) x DEFER(RECURSE_INDIRECT)() (x)
#define RECURSE_INDIRECT() RECURSE

#define EXPAND_TEST_EXISTS(...) EXPANDED, EXISTS(__VA_ARGS__) ) EAT (
#define GET_TEST_EXISTS_RESULT(x) ( GLUE_(EXPAND_TEST_, x), DOESNT_EXIST )

#define GET_TEST_EXIST_VALUE(exp, val) val

#define TEST_EXISTS(x) EVAL(DEFER(GET_TEST_EXIST_VALUE) ( GET_TEST_EXISTS_RESULT(x) ))

#define DOES_VALUE_EXIST_EXISTS(...) 1 
#define DOES_VALUE_EXIST_DOESNT_EXIST 0
#define DOES_VALUE_EXIST(x) GLUE_(DOES_VALUE_EXIST_, x)

#define EXTRACT_VALUE_EXISTS(...) __VA_ARGS__
#define EXTRACT_VALUE(value) GLUE_(EXTRACT_VALUE_, value)

#define IF_1(t, ...) t
#define IF_0(t, ...) __VA_ARGS__
#define IF(value) GLUE_(IF_, value)

#define TRY_EXTRACT_EXISTS(value, ...)            \
  IF ( DOES_VALUE_EXIST( TEST_EXISTS(value)) )    \
  ( EXTRACT_VALUE(value), __VA_ARGS__ )

#define AND_11 EXISTS(1)
#define AND(x, y) TRY_EXTRACT_EXISTS ( GLUE_(GLUE_(AND_, x), y), 0 )

#define OR_00 EXISTS(0)
#define OR(x, y) TRY_EXTRACT_EXISTS ( GLUE_(GLUE_(OR_, x), y), 1 )

#define XOR_01 EXISTS(1)
#define XOR_10 EXISTS(1)
#define XOR(x, y) TRY_EXTRACT_EXISTS ( GLUE_(GLUE_(XOR_, x), y), 0 )

#define NOT_0 EXISTS(1)
#define NOT(x) TRY_EXTRACT_EXISTS ( GLUE_(NOT_, x), 0 )

#define IS_ZERO_0 EXISTS(1)
#define IS_ZERO(x) TRY_EXTRACT_EXISTS ( GLUE_(IS_ZERO_, x), 0 )

#define IS_NOT_ZERO(x) NOT ( IS_ZER0(x) )

#define ADDI(x, y) 

#define HEAD(a, ...) a 
#define TAIL(a, ...) __VA_ARGS__

#define TEST_LAST EXISTS(1)
#define IS_LIST_EMPTY(...)                \
  TRY_EXTRACT_EXISTS (                    \
      DEFER(HEAD) (__VA_ARGS__ EXISTS(1)) \
      , 0)

#define IS_LIST_NOT_EMPTY(...) NOT(IS_LIST_EMPTY(__VA_ARGS__))

#define WRAP(...) ( __VA_ARGS__ )
#define UNWRAP_(...) __VA_ARGS__
#define UNWRAP(...) UNWRAP_ (__VA_ARGS__)

#define IF_WRAPPED_1(t, ...) t
#define IF_WRAPPED_0(t, ...) __VA_ARGS__
#define IF_WRAPPED(...) GLUE_(IF_WRAPPED_, IS_ENCLOSED(__VA_ARGS__))

#define OPT_UNWRAP(...) IF_WRAPPED(__VA_ARGS__) ( UNWRAP(__VA_ARGS__), __VA_ARGS__ )

#define FOR_EACH_INDIRECT() FOR_EACH_NO_EVAL
#define FOR_EACH_NO_EVAL(f, ...)                          \
  IF(IS_LIST_NOT_EMPTY(__VA_ARGS__)) (                    \
    f( OPT_UNWRAP(HEAD(__VA_ARGS__)))                     \
    DEFER2 (FOR_EACH_INDIRECT) () (f, TAIL(__VA_ARGS__))  \
    )

#define FOR_EACH(f, ...) \
  EVAL(FOR_EACH_NO_EVAL(f, __VA_ARGS__))

#define X_COMMA(x) x,

#define X_SEMICOLON(x) x;

#define ITERATOR_1    L_INDICES_TO_1    (X_COMMA) 
#define ITERATOR_2    L_INDICES_TO_2    (X_COMMA) 
#define ITERATOR_4    L_INDICES_TO_4    (X_COMMA) 
#define ITERATOR_8    L_INDICES_TO_8    (X_COMMA) 
#define ITERATOR_16   L_INDICES_TO_16   (X_COMMA)
#define ITERATOR_32   L_INDICES_TO_32   (X_COMMA)
#define ITERATOR_64   L_INDICES_TO_64   (X_COMMA)
#define ITERATOR_128  L_INDICES_TO_128  (X_COMMA)
#define ITERATOR_256  L_INDICES_TO_256  (X_COMMA)
#define ITERATOR_512  L_INDICES_TO_512  (X_COMMA)
#define ITERATOR_1024 L_INDICES_TO_1024 (X_COMMA)

#define ITERATE(f) FOR_EACH(f, ITERATOR_2)


//#define OUTPUT_RANGE_VALUE(x) x,
//#define RANGE_INDIRECT() RANGE_
//#define RANGE_(start, end, step) (\
//    IF( IS_NOT_ZERO(SUBI(end, start))
