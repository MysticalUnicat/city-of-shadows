#ifndef cpp_h_INCLUDED
#define cpp_h_INCLUDED

#define CPP_CAT(A, ...) CPP_CAT_(A, __VA_ARGS__)
#define CPP_CAT_(A, ...) A##__VA_ARGS__

#define CPP_CAT3(A, B, ...) CPP_CAT3_(A, B, __VA_ARGS__)
#define CPP_CAT3_(A, B, ...) A##B##__VA_ARGS__

#define CPP_CAT4(A, B, C, ...) CPP_CAT4_(A, B, C, __VA_ARGS__)
#define CPP_CAT4_(A, B, C, ...) A##B##C##__VA_ARGS__

#define CPP_CAT5(A, B, C, D, ...) CPP_CAT5_(A, B, C, D, __VA_ARGS__)
#define CPP_CAT5_(A, B, C, D, ...) A##B##C##D##__VA_ARGS__

#define CPP_CAT6(A, B, C, D, E, ...) CPP_CAT6_(A, B, C, D, E, __VA_ARGS__)
#define CPP_CAT6_(A, B, C, D, E, ...) A##B##C##D##E##__VA_ARGS__

#define CPP_EVAL CPP_EVAL_0
#define CPP_EVAL_0(...) CPP_EVAL_1(CPP_EVAL_1(CPP_EVAL_1(CPP_EVAL_1(__VA_ARGS__))))
#define CPP_EVAL_1(...) CPP_EVAL_2(CPP_EVAL_2(CPP_EVAL_2(CPP_EVAL_2(__VA_ARGS__))))
#define CPP_EVAL_2(...) CPP_EVAL_3(CPP_EVAL_3(CPP_EVAL_3(CPP_EVAL_3(__VA_ARGS__))))
#define CPP_EVAL_3(...) __VA_ARGS__

#define CPP_EMPTY()
#define CPP_DEFER_1(X) X CPP_EMPTY()
#define CPP_DEFER_2(X) X CPP_EMPTY CPP_EMPTY()()
#define CPP_DEFER_3(X) X CPP_EMPTY CPP_EMPTY CPP_EMPTY()()()

#define CPP_PROBE(...) ~, 1

#define CPP_IS_PROBE(...) CPP_IS_PROBE_(__VA_ARGS__, 0, 0)
#define CPP_IS_PROBE_(X, Y, ...) Y

#define CPP_BOOLEAN_AND(X, Y) CPP_CAT3(CPP_BOOLEAN_AND_, X, Y)
#define CPP_BOOLEAN_AND_00 0
#define CPP_BOOLEAN_AND_10 0
#define CPP_BOOLEAN_AND_01 0
#define CPP_BOOLEAN_AND_11 1

#define CPP_BOOLEAN_OR(X, Y) CPP_CAT3(CPP_BOOLEAN_OR_, X, Y)
#define CPP_BOOLEAN_OR_00 0
#define CPP_BOOLEAN_OR_10 1
#define CPP_BOOLEAN_OR_01 1
#define CPP_BOOLEAN_OR_11 1

#define CPP_BOOLEAN_NOT(X) CPP_CAT(CPP_BOOLEAN_NOT_, X)
#define CPP_BOOLEAN_NOT_0 1
#define CPP_BOOLEAN_NOT_1 0

#define CPP_BOOLEAN_IMPLY(X, Y) CPP_CAT3(CPP_BOOLEAN_IMPLY_, X, Y)
#define CPP_BOOLEAN_IMPLY_00 1
#define CPP_BOOLEAN_IMPLY_10 0
#define CPP_BOOLEAN_IMPLY_01 1
#define CPP_BOOLEAN_IMPLY_11 1

#define CPP_BOOLEAN_XOR(X, Y) CPP_CAT3(CPP_BOOLEAN_XOR_, X, Y)
#define CPP_BOOLEAN_XOR_00 0
#define CPP_BOOLEAN_XOR_10 1
#define CPP_BOOLEAN_XOR_01 1
#define CPP_BOOLEAN_XOR_11 0

#define CPP_BOOLEAN_EQ(X, Y) CPP_CAT3(CPP_BOOLEAN_EQ_, X, Y)
#define CPP_BOOLEAN_EQ_00 1
#define CPP_BOOLEAN_EQ_10 0
#define CPP_BOOLEAN_EQ_01 0
#define CPP_BOOLEAN_EQ_11 1

#define CPP_BOOLEAN_CHOICE(X) CPP_CAT(CPP_BOOLEAN_CHOICE_, X)
#define CPP_BOOLEAN_CHOICE_0(...)              CPP_BOOLEAN_CHOICE_0_
#define CPP_BOOLEAN_CHOICE_0_(...) __VA_ARGS__
#define CPP_BOOLEAN_CHOICE_1(...)  __VA_ARGS__ CPP_BOOLEAN_CHOICE_1_
#define CPP_BOOLEAN_CHOICE_1_(...) 

#define CPP_ADD  CPP_BOOLEAN_AND
#define CPP_OR   CPP_BOOLEAN_OR
#define CPP_BNOT CPP_BOOLEAN_NOT

#define CPP_IFF(X) CPP_CAT(CPP_IFF_, X)
#define CPP_IFF_0(T, F) F
#define CPP_IFF_1(T, F) T

// test<x,y>( 1 )( 0 )
#define CPP_IF1 CPP_BOOLEAN_CHOICE 

// test<x,y>( 11 )( 10 )( 01 )( 00 )
#define CPP_IF2(X, Y) CPP_CAT4(CPP_IF2_, X, Y, _0)

#define CPP_IF2_11_0(...) __VA_ARGS__ CPP_IF2_11_1
#define CPP_IF2_11_1(...)             CPP_IF2_11_2
#define CPP_IF2_11_2(...)             CPP_IF2_11_3
#define CPP_IF2_11_3(...) 

#define CPP_IF2_10_0(...)             CPP_IF2_10_1
#define CPP_IF2_10_1(...) __VA_ARGS__ CPP_IF2_10_2
#define CPP_IF2_10_2(...)             CPP_IF2_10_3
#define CPP_IF2_10_3(...) 

#define CPP_IF2_01_0(...)             CPP_IF2_01_1
#define CPP_IF2_01_1(...)             CPP_IF2_01_2
#define CPP_IF2_01_2(...) __VA_ARGS__ CPP_IF2_01_3
#define CPP_IF2_01_3(...) 

#define CPP_IF2_00_0(...)             CPP_IF2_00_1
#define CPP_IF2_00_1(...)             CPP_IF2_00_2
#define CPP_IF2_00_2(...)             CPP_IF2_00_3
#define CPP_IF2_00_3(...) __VA_ARGS__ 

#define CPP_EQ(PREFIX, X, Y) CPP_IS_PROBE(CPP_CAT5(CPP_EQ__, PREFIX, X, _, Y)())

// simple template for how to use CPP_EQ
#define CPP_EQ__CPP0_0 CPP_PROBE

// used here
#define CPP_IS_ZERO(X) CPP_EQ(CPP, 0, X)

#define CPP_IS_NONZERO(X) CPP_BNOT(CPP_IS_ZERO(X))

#define CPP_IS_BEGIN_PAREN(X) CPP_IS_BEGIN_PAREN_(CPP_IS_BEGIN_PAREN_call X)
#define CPP_IS_BEGIN_PAREN_(X) CPP_IS_PROBE(X())
#define CPP_IS_BEGIN_PAREN_call(...) CPP_PROBE

// create Some iff X is 1, or else None
#define CPP_Some_IFF(X) CPP_CAT(CPP_Some_IFF_test_, X)
#define CPP_Some_IFF_test_0(...) None()
#define CPP_Some_IFF_test_1(...) Some(__VA_ARGS__)

// tell us if this is an option! can take any identifier or number-pp token
#define CPP_IS_OPTION(OPTION) CPP_IS_PROBE(CPP_CAT(IS_OPTION_, OPTION))
#define CPP_IS_OPTION_Some(...) CPP_PROBE()
#define CPP_IS_OPTION_None(...) CPP_PROBE()

// the Option's type None or Some
#define CPP_OPTION_TYPE(OPTION) CPP_CAT(CPP_OPTION_TYPE_, OPTION)
#define CPP_OPTION_TYPE_Some(...) Some
#define CPP_OPTION_TYPE_None(...) None

// same as OPTION_TYPE but returns 1/0
#define CPP_OPTION_IS_SOME(OPTION) CPP_CAT(CPP_OPTION_IS_SOME_, OPTION)
#define CPP_OPTION_IS_SOME_Some(...) 1
#define CPP_OPTION_IS_SOME_None(...) 0

#define CPP_OPTION_IS_NONE(OPTION) CPP_BNOT(CPP_OPTION_IS_SOME(OPTION))

// the Option's value as a parameter list.
// None always outputs ~ to cause syntax errors if the value is ever attempted to be emitted
#define CPP_OPTION_VALUE(OPTION) CPP_CAT(CPP_OPTION_VALUE_, OPTION)
#define CPP_OPTION_VALUE_Some(...) ( __VA_ARGS__ )
#define CPP_OPTION_VALUE_None(...) ~

// Simple map operation for option
#define CPP_OPTION_MAP(OPTION, F)      \
  CPP_IFF(                             \
    CPP_OPTION_IS_SOME(OPTION)         \
  )(                                     \
      Some(F CPP_OPTION_VALUE(OPTION)) \
    , None()                             \
  )

// unwrap an option, with a defualt value for None()
#define CPP_OPTION_UNWRAP(OPTION, DEFAULT) \
  CPP_IFF( \
    CPP_OPTION_IS_SOME(OPTION) \
  )( \
      CPP_EVAL CPP_OPTION_VALUE(OPTION) \
    , DEFAULT \
  )

#define CPP_PRIMITIVE_1st(A, ...) A
#define CPP_PRIMITIVE_2nd(A, B, ...) B
#define CPP_PRIMITIVE_3rd(A, B, C, ...) C

// get the first argument, hopefully it catches a lot of things, returns None() or Some(X)
#define CPP_FIRST(...) CPP_FIRST_(~, ## __VA_ARGS__, CPP_PROBE)
#define CPP_FIRST_(X, Y, ...) CPP_Some_IFF(CPP_BNOT(CPP_IS_PROBE(Y())))(Y)

#define CPP_UNWRAP(X) CPP_DEFER_1(CPP_ALL) X

#define CPP_MAP(F, X, ...) \
  F(X) \
  CPP_IFF( \
    CPP_OPTION_IS_SOME(CPP_FIRST(__VA_ARGS__)) \
  )( \
      CPP_DEFER_2(CPP_MAP_ID)()(F, __VA_ARGS__) \
    , /* do nothing */ \
  )
#define CPP_MAP_ID() CPP_MAP

#define CPP_MAP2(F, X, Y, ...) \
  F(X, Y) \
  CPP_IFF( \
    CPP_OPTION_IS_SOME(CPP_FIRST(__VA_ARGS__)) \
  )( \
      CPP_DEFER_2(CPP_MAP2_ID)()(F, X, __VA_ARGS__) \
    , /* do nothing */ \
  )
#define CPP_MAP2_ID() CPP_MAP2

#define CPP_FILTER_MAP(P, F, X, ...) \
  CPP_IFF(P(X))(F(X),) \
  CPP_IFF( \
    CPP_OPTION_IS_SOME(CPP_FIRST(__VA_ARGS__)) \
  )( \
      CPP_DEFER_2(CPP_FILTER_MAP_ID)()(P, F, __VA_ARGS__) \
    , /* do nothing */ \
  )
#define CPP_FILTER_MAP_ID() CPP_FILTER_MAP

#define CPP_FILTER_ANY(P, X, ...) \
  CPP_IFF(P(X))(1, \
  CPP_IFF( \
    CPP_OPTION_IS_SOME(CPP_FIRST(__VA_ARGS__)) \
  )( \
      CPP_DEFER_2(CPP_FILTER_ANY_ID)()(P, __VA_ARGS__) \
    , 0 \
  ))
#define CPP_FILTER_ANY_ID() CPP_FILTER_ANY

#endif // cpp_h_INCLUDED
