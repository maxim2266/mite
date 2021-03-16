#include "mite.h"

TEST_CASE(test_case_1)
{
	TEST(printf("### %s [%s]\n", __func__, __FILE__) > 0);
}

TEST_CASE(test_case_2)
{
	TEST(printf("### %s [%s]\n", __func__, __FILE__) > 0);
}

TEST_CASE(test_case_3)
{
	TEST(printf("### %s [%s]\n", __func__, __FILE__) > 0);
}

TEST_CASE(test_case_4)
{
	TEST(printf("### %s [%s]\n", __func__, __FILE__) > 0);
}
