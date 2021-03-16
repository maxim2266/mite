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

static _test_rec *first = NULL, *last = NULL;
static unsigned count = 0;

void _register_test(_test_rec* rec)
{
	++count;
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

int main(void)
{
	if(!first)
	{
		fputs("No test to run.", stderr);
		return 1;
	}

	printf("Running %u test cases:\n", count);
	flush_out();

	for(const _test_rec* p = first; p; p = p->next)
	{
		const clock_t ts = clock();

		p->fn();

		printf("passed: %s [%s] in %fs\n",
			   p->name, p->file_name, ((double)(clock() - ts)) / CLOCKS_PER_SEC);

		flush_out();
	}

	puts("All passed.");
	flush_out();

	return 0;
}
