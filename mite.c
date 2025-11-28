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

// test case registration
static mite_test_case *first = NULL, *last = NULL;

void mite_register_test(mite_test_case* rec)
{
	rec->next = NULL;

	if(!first)
		first = last = rec;
	else
		last = last->next = rec;
}

// termination functions
static unsigned num_failures = 0;

static void do_exit(void) 		{ exit(1); }
static void do_nothing(void)	{ ++num_failures; }

void (*mite_failed)(void) = do_exit;

// filter test cases
static int all_pass(const mite_test_case* const rec __attribute__((unused)))	{ return 1; }

static int (*accept_test_case)(const mite_test_case* const) = all_pass;

// regex
static regex_t regex;

static
void free_regex(void)
{
	regfree(&regex);
}

static __attribute__((noreturn))
void exit_with_regex_error(const int err)
{
	char buff[4096];

	regerror(err, &regex, buff, sizeof(buff));
	fprintf(stderr, "error in filter pattern: %s\n", buff);
	exit(1);
}

// regex matching
static
int match_regex(const mite_test_case* const rec)
{
	return regexec(&regex, rec->name, 0, NULL, 0) == 0;
}

// regex compilation
static
void compile_regex(const char* const re)
{
	const int err = regcomp(&regex, re, REG_NOSUB | REG_EXTENDED);

	if(err)
		exit_with_regex_error(err);

	atexit(free_regex);
	accept_test_case = match_regex;
}

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

// option parsing
static
void parse_options(const char** argv)
{
	const char* const prog_name = *argv++;
	int got_filter = 0;

	while(*argv)
	{
		const char* arg = *argv++;

		if(strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0)
			usage_exit(prog_name);

		if(strcmp(arg, "-a") == 0 || strcmp(arg, "--all") == 0)
		{
			mite_failed = do_nothing;
			continue;
		}

		if(strcmp(arg, "-f") == 0 || strcmp(arg, "--filter") == 0)
		{
			if(got_filter)
			{
				fputs("error: only one filter may be given\n\n", stderr);
				usage_exit(prog_name);
			}

			if(!(arg = *argv++))
			{
				fputs("error: missing filter pattern\n\n", stderr);
				usage_exit(prog_name);
			}

			compile_regex(arg);
			got_filter = 1;
			continue;
		}

		fprintf(stderr, "error: unknown parameter \"%s\"\n\n", arg);
		usage_exit(prog_name);
	}
}

// timing
static inline
double time_since(const clock_t start)
{
	return ((double)(clock() - start)) / CLOCKS_PER_SEC;
}

// flush STDOUT
static
void flush_out(void)
{
	if(fflush(stdout) == EOF)
	{
		perror("error writing to STDOUT");
		exit(2);
	}
}

// entry point for the test binary
int main(int argc __attribute__((unused)), const char* argv[])
{
	parse_options(argv);

	if(!first)
	{
		fputs("No test to run.\n", stderr);
		return 1;
	}

	unsigned count = 0;
	const clock_t start = clock();

	for(const mite_test_case* p = first; p; p = p->next)
	{
		if(!accept_test_case(p))
			continue;

		++count;

		const clock_t ts = clock();

		p->fn();
		printf("passed: %s [%s] in %fs\n", p->name, p->file_name, time_since(ts));
		flush_out();
	}

	printf("Done: %u test cases (%u failed) in %fs.\n", count, num_failures, time_since(start));
	flush_out();

	return num_failures > 0;
}
