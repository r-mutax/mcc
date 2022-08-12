#define TST_PREPROCESS_INC_H OK_TEST_DEFINE
int OK_TEST_DEFINE;

// test ifdef directive.
#ifdef TST_PREPROCESS_INC_H
int OK_TEST_IFDEF_DIRECTIVE;
#else
int NG_TEST_IFDEF_DIRECTIVE;
#endif

// test ifndef directive.
#ifndef IFNDEF_TEST_DEFINE
int OK_TEST_IFNDEF_DIRECTIVE;
#else
int NG_TEST_IFNDEF_DIRECTIVE;
#endif

// test #undef directive.
#undef TST_PREPROCESS_INC_H
int TST_PREPROCESS_INC_H;

// test #if directive.
#if 0
int NG_TEST_IF_WITH;
#else

// test Nesting #if directive.
#if 0
int NG_TEST_NESTING_IF;
#else
int OK_TEST_NESTING_IF;
#endif
#endif

// test #if - #elif - #endif directive.
#if 0
int NG_TEST_IF_ELIF_ENDIF;
#elif 0
int NG_TEST_IF_ELIF_ENDIF;
#elif 1
int OK_TEST_IF_ELIF_ENDIF;
#endif

// test #if directive expression
#if (2 * 3 - 6)
int NG_TEST_IF_EXPR_Multiple;
#endif

#if (4 / 2 + 4) - 6
int NG_TEST_IF_EXPR_Division;
#endif

#if (5 % 3 + 1) - 3
int NG_TEST_IF_EXPR_Mod;
#endif

#if (1 << 3 + 2) - 32
int NG_TEST_IF_EXPR_RightBitShift;
#endif

#if (8 >> 3 - 2) - 4
int NG_TEST_IF_EXPR_LeftBitShift;
#endif

#if (7 < 4 << 1) - 1
int NG_TEST_IF_EXPR_Less_Than;
#endif

#if (2 <= 8 >> 2) - 1
int NG_TEST_IF_EXPR_Less_Equal;
#endif

#if (17 > 4 << 2) - 1
int NG_TEST_IF_EXPR_Greater_than;
#endif

#if (1 >= 4 >> 2) - 1
int NG_TEST_IF_EXPR_Greater_Equal;
#endif

#if (1 == 2 > 0 != 2) - 1
int NG_TEST_IF_EXPR_Equal_and_NotEqual;
#endif

#if (2 & 5 ^ 3 | 9) - 11
int NG_TEST_IF_EXPR_bitAnd_bitXor_bitOr;
#endif

#if (3 == 3 && 4 != 4 ? 3 : 9) - 9
int NG_TEST_IF_EXPR_Condition_Expr;
#endif

#define TEST_DEF 1
#if defined ( TEST_DEF ) - 1
int NG_TEST_IF_EXPR_defined_val;
#endif

#if ! defined TEST_DEF
int NG_TEST_IF_EXPR_defined;
#endif

#define add(a, b) a + b

int TEST_FUNCLIKE_MACRO = add(3, 5);