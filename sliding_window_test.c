#include <stdio.h>
#include "sliding_window.h"

#define COL_NORLAL "\033[0;0m"
#define COL_SUCCESS "\033[0;32m"
#define COL_FAIL "\033[0;31m"

#define WIDTH 20

#define ARRAY_SZ(arr) (sizeof(arr)/sizeof(arr[0]))
#define TEST_SEQUENCE(sw, seq) test_sequence(sw, seq, ARRAY_SZ(seq), \
	seq ## _results)

struct sw_test {
	enum sw_val dir;
	u32 count;
};

static void print_result(char *str, u32 result, u32 expected)
{
	int ok = result == expected;

	printf("total %s = %u (%s%s%s)\n", str, result,
		ok ? COL_SUCCESS : COL_FAIL, ok ? "good" : "bad", COL_NORLAL);
}

static void test_sequence(struct sliding_window *sw, struct sw_test *sequence,
	int len, u32 results[3])
{
	int i;

	for (i = 0; i < len; i++) {
		int j;

		for (j = 0; j < sequence[i].count; j++)
			sw_advance(sw, sequence[i].dir);
	}

	printf("\n");
	print_result("SW_NONE", sw_val_get(sw, SW_NONE), results[SW_NONE]);
	print_result("SW_READ", sw_val_get(sw, SW_READ), results[SW_READ]);
	print_result("SW_WRITE", sw_val_get(sw, SW_WRITE), results[SW_WRITE]);
}

int main(int argc, char **argv)
{
	struct sliding_window sw;
	struct sw_test sequence1[] = {
		{ SW_WRITE, 15 },
		{ SW_READ, 1 },
		{ SW_WRITE, 5 },
		{ SW_READ, 3 },
	};
	u32 sequence1_results[3] = {
		[ SW_NONE ] = 0,
		[ SW_READ ] = 4,
		[ SW_WRITE ] =16 
	};

	struct sw_test sequence2[] = {
		{ SW_WRITE, 5 },
		{ SW_WRITE, 34 },
		{ SW_READ, 3 },
		{ SW_WRITE, 7 },
		{ SW_READ, 4 },
	};
	u32 sequence2_results[3] = {
		[ SW_NONE ] = 0,
		[ SW_READ ] = 7,
		[ SW_WRITE ] =13 
	};

	struct sw_test sequence3[] = {
		{ SW_WRITE, 5 },
		{ SW_WRITE, 17 },
		{ SW_NONE, 3 },
		{ SW_WRITE, 2 },
		{ SW_READ, 4 },
	};
	u32 sequence3_results[3] = {
		[ SW_NONE ] = 3,
		[ SW_READ ] = 4,
		[ SW_WRITE ] =13 
	};

	printf("sw_init(&sw, %d): %d\n", WIDTH, sw_init(&sw, WIDTH));
	printf("sw_widthget_(&sw): %u\n", sw_width_get(&sw));

	TEST_SEQUENCE(&sw, sequence1);
	sw_reset(&sw);
	TEST_SEQUENCE(&sw, sequence2);
	sw_reset(&sw);
	TEST_SEQUENCE(&sw, sequence3);

	sw_uninit(&sw);
	return 0;
}

