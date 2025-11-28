/*
BSD 3-Clause License

Copyright (c) 2021, Maxim Konakov
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

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
		mite_failed();	\
		return;	\
	}	\
})

#define TEST(cond)	TESTF((cond), "`%s`", #cond)

#define TEST_CASE(name)	\
	static void name(void);	\
	static __attribute__((constructor)) void _cons_ ## name(void)	\
	{	\
		static mite_test_case rec = { name, #name, __FILE__, NULL };	\
		mite_register_test(&rec);	\
	}	\
	static void name(void)

// implementation
typedef struct mite_test_case
{
	void (*fn)(void);
	const char *name, *file_name;
	struct mite_test_case* next;
} mite_test_case;

void mite_register_test(mite_test_case*) __attribute__((nonnull));

extern void (*mite_failed)(void);

#ifdef __cplusplus
}
#endif
