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

// #error this is error.