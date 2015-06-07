#include <stdio.h>
#include "sliding_window.h"

#define COL_NORMAL "\033[0;0m"
#define COL_SUCCESS "\033[0;32m"
#define COL_FAIL "\033[0;31m"

#define WIDTH 20

#define ARRAY_SZ(arr) (sizeof(arr)/sizeof(arr[0]))

#define PRINT_RESULT(sw, test, sw_val) print_result(#sw_val, \
	sw_val_get(sw, sw_val), test->expected[sw_val])

struct sw_sequence {
	enum sw_val dir;
	u32 count;
};

struct sw_test {
	struct sw_sequence *seq;
	int len;
	u32 expected[3];
};

static void print_result(char *str, u32 result, u32 expected)
{
	int ok = result == expected;

	printf("total %s = %u (%s%s%s)\n", str, result,
		ok ? COL_SUCCESS : COL_FAIL, ok ? "good" : "bad", COL_NORMAL);
}

static void test_sequence(struct sliding_window *sw, struct sw_test *test)
{
	int i;

	for (i = 0; i < test->len; i++) {
		int j;

		for (j = 0; j < test->seq[i].count; j++)
			sw_advance(sw, test->seq[i].dir);
	}

	printf("\n");
	PRINT_RESULT(sw, test, SW_NONE);
	PRINT_RESULT(sw, test, SW_READ);
	PRINT_RESULT(sw, test, SW_WRITE);
}

int main(int argc, char **argv)
{
	int err;
	u32 width;
	struct sliding_window sw;
	struct sw_sequence sequence1[] = {
		{ SW_WRITE, 15 },
		{ SW_READ, 1 },
		{ SW_WRITE, 5 },
		{ SW_READ, 3 },
	};
	struct sw_test t1 = {
		.seq = sequence1,
		.len = ARRAY_SZ(sequence1),
		.expected = {
			[ SW_NONE ] = 0, [ SW_READ ] = 4, [ SW_WRITE ] = 16 
		}
	};

	struct sw_sequence sequence2[] = {
		{ SW_WRITE, 5 },
		{ SW_WRITE, 34 },
		{ SW_READ, 3 },
		{ SW_WRITE, 7 },
		{ SW_READ, 4 },
	};
	struct sw_test t2 = {
		.seq = sequence2,
		.len = ARRAY_SZ(sequence2),
		.expected = {
			[ SW_NONE ] = 0, [ SW_READ ] = 7, [ SW_WRITE ] = 13 
		}
	};

	struct sw_sequence sequence3[] = {
		{ SW_WRITE, 5 },
		{ SW_WRITE, 17 },
		{ SW_NONE, 3 },
		{ SW_WRITE, 2 },
		{ SW_READ, 4 },
	};
	struct sw_test t3 = {
		.seq = sequence3,
		.len = ARRAY_SZ(sequence3),
		.expected = {
			[ SW_NONE ] = 3, [ SW_READ ] = 4, [ SW_WRITE ] = 13 
		}
	};

	err = sw_init(&sw, WIDTH);
	printf("sw_init(&sw, %d): %d (%s%s%s)\n", WIDTH, err,
		err ? COL_FAIL : COL_SUCCESS, err ? "bad" : "good", COL_NORMAL);
	width = sw_width_get(&sw);
	printf("sw_widthget_(&sw): %u (%s%s%s)\n", sw_width_get(&sw),
		width != WIDTH ? COL_FAIL : COL_SUCCESS, err ? "bad" : "good",
		COL_NORMAL);

	test_sequence(&sw, &t1);
	sw_reset(&sw);
	test_sequence(&sw, &t2);
	sw_reset(&sw);
	test_sequence(&sw, &t3);

	sw_uninit(&sw);
	return 0;
}

