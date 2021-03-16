# mite: minimalist test framework for C language.

[![License: BSD 3 Clause](https://img.shields.io/badge/License-BSD_3--Clause-yellow.svg)](https://opensource.org/licenses/BSD-3-Clause)

### Usage

Add file `mite.c` to the target project and include `mite.h` header into every test source.
The framework defines its own `main` function, so the file with the project's `main` should
be excluded from the test set-up.

Test case example:
```C
#include "mite.h"

TEST_CASE(my_test_case)
{
	...
	TEST(some_ptr != NULL);
	TESTF(some_value > 0, "invalid value %d", some_value);
	...
}
```

#### Status
Tested on Linux Mint 20.1.
