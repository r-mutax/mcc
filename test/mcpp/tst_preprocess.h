#define TST_PREPROCESS_INC_H OK_TEST_DEFINE
int OK_TEST_DEFINE;

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