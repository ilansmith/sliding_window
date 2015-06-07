#ifndef _SLIDING_WINDOW_H_
#define _SLIDING_WINDOW_H_

#include "sliding_window_compat.h"

typedef unsigned int u32;
typedef unsigned long spinlock_t;

enum sw_val {
	SW_NONE = 0,
	SW_READ,
	SW_WRITE
};

struct sliding_window {
	u32 *__window;	/* initialized to some given size */
	u32 __size;	/* size of the sliding window in words */
	u32 *__major;	/* pointer into window */
	u32 __offset;	/* offset of the 0x3 mask in __major */
	u32 width;	/* sliding window width */
	u32 stat[2];	/* statistic table */
	spinlock_t lock;
};

int sw_alloc(struct sliding_window *sw, u32 size);
void sw_free(struct sliding_window *sw);

bool sw_advance(struct sliding_window *sw, enum sw_val val);
void sw_reset(struct sliding_window *sw);

u32 sw_width_get(struct sliding_window *sw);
u32 sw_val_get(struct sliding_window *sw, enum sw_val val);
#endif

