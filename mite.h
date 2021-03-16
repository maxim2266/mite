#pragma once

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// (precaution) erase previous definitions, to avoid name clashes with other frameworks
#ifdef TEST
#undef TEST
#endif

#ifdef TESTF
#undef TESTF
#endif

#ifdef TEST_CASE
#undef TEST_CASE
#endif

// macro definitions
#define TESTF(cond, fmt, ...)	\
({	\
	if(!(cond))	\
	{	\
		fprintf(stderr, "FAILED: %s [%s:%u]: " fmt "\n", __func__, __FILE__, __LINE__, ##__VA_ARGS__);	\
		exit(1);	\
	}	\
})

#define TEST(cond)	TESTF((cond), "`%s`", #cond)

#define TEST_CASE(name)	\
	static void name(void);	\
	static __attribute__((constructor)) void _cons_ ## name(void)	\
	{	\
		static _test_rec rec = (_test_rec){ __FILE__, __LINE__, #name, name, NULL };	\
		_register_test(&rec);	\
	}	\
	static void name(void)

// implementation
typedef struct _test_rec
{
	const char* const file_name;
	const int line;
	const char* const name;
	void (*fn)(void);
	struct _test_rec* next;
} _test_rec;

void _register_test(_test_rec*) __attribute__((nonnull));

#ifdef __cplusplus
}
#endif
