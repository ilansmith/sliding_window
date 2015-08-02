#include <stdio.h>
#include "sliding_window.h"

#define COL_NORMAL "\033[0;0m"
#define COL_SUCCESS "\033[0;32m"
#define COL_FAIL "\033[0;31m"

#define WIDTH 20

#define ARRAY_SZ(arr) (sizeof(arr)/sizeof(arr[0]))

#define PRINT_RESULT(slw, test, slw_val) print_result(#slw_val, \
	slw_val_get(slw, slw_val), test->expected[slw_val])

enum slw_op {
	SLW_OP_IO,
	SLW_OP_RESIZE,
};

struct slw_io {
	enum slw_val dir;
	u32 count;
};

struct slw_resize {
	u32 sz;
};

struct slw_sequence {
	union {
		struct slw_io io;
		struct slw_resize resize;
	} action;
	enum slw_op op;
};

struct slw_test {
	struct slw_sequence *seq;
	int len;
	u32 expected[3];
};

static void print_result(char *str, u32 result, u32 expected)
{
	int ok = result == expected;

	printf("total %s = %u (%s%s%s)\n", str, result,
		ok ? COL_SUCCESS : COL_FAIL, ok ? "good" : "bad", COL_NORMAL);
}

static void test_sequence(struct sliding_window *slw, struct slw_test *test)
{
	int i;
	int reinstate_size = 0;

	for (i = 0; i < test->len; i++) {
		int j;

		switch (test->seq[i].op) {
		case SLW_OP_IO:
			for (j = 0; j < test->seq[i].action.io.count; j++)
				slw_advance(slw, test->seq[i].action.io.dir);
			break;
		case SLW_OP_RESIZE:
			slw_resize(slw, test->seq[i].action.resize.sz);
			reinstate_size =
				test->seq[i].action.resize.sz == WIDTH ? 0 : 1;
			break;
		default:
			break;
		}
	}

	printf("\n");
	PRINT_RESULT(slw, test, SLW_NONE);
	PRINT_RESULT(slw, test, SLW_READ);
	PRINT_RESULT(slw, test, SLW_WRITE);

	if (reinstate_size)
		slw_resize(slw, WIDTH);
	slw_reset(slw);
}

int main(int argc, char **argv)
{
	int i, err;
	u32 width;
	struct sliding_window slw;

	struct slw_sequence sequence1[] = {
		{ .action.io = { SLW_WRITE, 15 }, .op = SLW_OP_IO },
		{ .action.io = { SLW_READ, 1 }, .op = SLW_OP_IO },
		{ .action.io = { SLW_WRITE, 5 }, .op = SLW_OP_IO },
		{ .action.io = { SLW_READ, 3 }, .op = SLW_OP_IO },
	};
	struct slw_test t1 = {
		.seq = sequence1,
		.len = ARRAY_SZ(sequence1),
		.expected = {
			[ SLW_NONE ] = 0, [ SLW_READ ] = 4, [ SLW_WRITE ] = 16 
		}
	};

	struct slw_sequence sequence2[] = {
		{ .action.io = { SLW_WRITE, 5 }, .op = SLW_OP_IO },
		{ .action.io = { SLW_WRITE, 34 }, .op = SLW_OP_IO },
		{ .action.io = { SLW_READ, 3 }, .op = SLW_OP_IO },
		{ .action.io = { SLW_WRITE, 7 }, .op = SLW_OP_IO },
		{ .action.io = { SLW_READ, 4 }, .op = SLW_OP_IO },
	};
	struct slw_test t2 = {
		.seq = sequence2,
		.len = ARRAY_SZ(sequence2),
		.expected = {
			[ SLW_NONE ] = 0, [ SLW_READ ] = 7, [ SLW_WRITE ] = 13 
		}
	};

	struct slw_sequence sequence3[] = {
		{ .action.io = { SLW_WRITE, 5 }, .op = SLW_OP_IO },
		{ .action.io = { SLW_WRITE, 17 }, .op = SLW_OP_IO },
		{ .action.io = { SLW_NONE, 3 }, .op = SLW_OP_IO },
		{ .action.io = { SLW_WRITE, 2 }, .op = SLW_OP_IO },
		{ .action.io = { SLW_READ, 4 }, .op = SLW_OP_IO },
	};
	struct slw_test t3 = {
		.seq = sequence3,
		.len = ARRAY_SZ(sequence3),
		.expected = {
			[ SLW_NONE ] = 3, [ SLW_READ ] = 4, [ SLW_WRITE ] = 13 
		}
	};

	struct slw_test *tests[] = { &t1, &t2, &t3 };

	err = slw_init(&slw, WIDTH);
	printf("slw_init(&slw, %d): %d (%s%s%s)\n", WIDTH, err,
		err ? COL_FAIL : COL_SUCCESS, err ? "bad" : "good", COL_NORMAL);
	width = slw_width_get(&slw);
	printf("slw_widthget_(&slw): %u (%s%s%s)\n", slw_width_get(&slw),
		width != WIDTH ? COL_FAIL : COL_SUCCESS, err ? "bad" : "good",
		COL_NORMAL);

	for (i = 0; i < ARRAY_SZ(tests); i++)
		test_sequence(&slw, tests[i]);

	slw_uninit(&slw);
	return 0;
}

