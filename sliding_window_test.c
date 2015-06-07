#include <stdio.h>
#include "sliding_window.h"

int main(int argc, char **argv)
{
	struct sliding_window sw;
	int ret, i;
	struct {
		enum sw_val dir;
		u32 count;
	} sequence[] = {
		{ SW_WRITE, 25 },
		{ SW_READ, 2 },
		{ SW_WRITE, 3 },
	};

	ret = sw_alloc(&sw, 2);
	printf("sw_alloc(&sw, 2): %d\n", ret);
	printf("sw_get_size(&sw): %u*sizeof(u32) = %lu (%lu bits)\n",
		sw_get_size(&sw), sw_get_size(&sw) * sizeof(u32),
		sw_get_size(&sw) * sizeof(u32) * 8);

	for (i = 0; i < ARRAY_SZ(sequence); i++) {
		int j;

		for (j = 0; j < sequence[i].count; j++)
			sw_advance(&sw, sequence[i].dir);
	}

	printf("total SW_READ = %u\n", sw_val_get(&sw, SW_READ));
	printf("total SW_WRITE = %u\n", sw_val_get(&sw, SW_WRITE));
	return 0;
}

