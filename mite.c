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
void do_flush(void)
{
	if(fflush(stdout) == EOF)
	{
		perror("error writing to STDOUT");
		exit(1);
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
	do_flush();

	for(const _test_rec* p = first; p; p = p->next)
	{
		const clock_t ts = clock();

		p->fn();

		printf("passed: %s [%s] in %fs\n",
			   p->name, p->file_name, ((double)(clock() - ts)) / CLOCKS_PER_SEC);

		do_flush();
	}

	puts("All passed.");
	do_flush();

	return 0;
}
