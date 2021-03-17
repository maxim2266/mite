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

#include "mite.h"

#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <regex.h>

static _test_case *first = NULL, *last = NULL;

void _register_test(_test_case* rec)
{
	rec->next = NULL;

	if(!first)
		first = last = rec;
	else
		last = last->next = rec;
}

static
void flush_out(void)
{
	if(fflush(stdout) == EOF)
	{
		perror("error writing to STDOUT");
		exit(2);
	}
}

// termination fucntions
static unsigned num_failures = 0;

static void do_exit(void) 		{ exit(1); }
static void do_nothing(void)	{ ++num_failures; }

void (*_failed)(void) = do_exit;

// show usage string
static __attribute__((noreturn))
void usage_exit(const char* prog_name)
{
	const char* const s = strrchr(prog_name, '/');

	if(s)
		prog_name = s + 1;

	fprintf(stderr,
			"Usage: %s [OPTION]...\n\n"
			"Options:\n"
			"  -a, --all              do not stop at the first failed test\n"
			"  -f, --filter PATTERN   run only the tests matching the given\n"
			"                         regular expression PATTERN\n"
			"  -h, --help             show this message and exit\n",
		    prog_name);
	exit(2);
}

// filter test cases
static int all_pass(const _test_case* const rec __attribute__((unused)))	{ return 1; }

static int (*accept_test_case)(const _test_case* const) = all_pass;

// regex
static regex_t regex;

static
void free_regex(void)
{
	regfree(&regex);
}

static __attribute__((noreturn))
void exit_with_regex_error(int err)
{
	char buff[4096];

	regerror(err, &regex, buff, sizeof(buff));
	fprintf(stderr, "error in filter pattern: %s\n", buff);
	exit(1);
}

// regex matching
static
int match_regex(const _test_case* const rec)
{
	return regexec(&regex, rec->name, 0, NULL, 0) == 0;
}

// regex compilation
static
void compile_regex(const char* const re)
{
	int err = regcomp(&regex, re, REG_NOSUB | REG_EXTENDED);

	if(err)
		exit_with_regex_error(err);

	atexit(free_regex);
	accept_test_case = match_regex;
}

// option parsing
static
void parse_options(int argc, char** argv)
{
	int re_compiled = 0;

	for(int i = 1; i < argc; ++i)
	{
		if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
			usage_exit(argv[0]);

		if(strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--all") == 0)
		{
			_failed = do_nothing;
			continue;
		}

		if(strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--filter") == 0)
		{
			if(re_compiled)
			{
				fputs("error: only one filter may be given\n\n", stderr);
				usage_exit(argv[0]);
			}

			if(++i == argc)
			{
				fputs("error: missing regular expression pattern\n\n", stderr);
				usage_exit(argv[0]);
			}

			compile_regex(argv[i]);
			re_compiled = 1;
			continue;
		}

		fprintf(stderr, "error: unknown parameter \"%s\"\n\n", argv[i]);
		usage_exit(argv[0]);
	}
}

// entry point for the test binary
int main(int argc, char** argv)
{
	parse_options(argc, argv);

	if(!first)
	{
		fputs("No test to run.\n", stderr);
		return 1;
	}

	unsigned count = 0;

	for(const _test_case* p = first; p; p = p->next)
	{
		if(!accept_test_case(p))
			continue;

		++count;

		const clock_t ts = clock();

		p->fn();

		printf("passed: %s [%s] in %fs\n",
			   p->name, p->file_name, ((double)(clock() - ts)) / CLOCKS_PER_SEC);

		flush_out();
	}

	printf("Done: %u test cases (%u failed).\n", count, num_failures);
	flush_out();

	return num_failures > 0;
}
