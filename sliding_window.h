#ifndef _SLIDING_WINDOW_H_
#define _SLIDING_WINDOW_H_

#include "sliding_window_compat.h"

#define ARRAY_SZ(arr) (sizeof(arr)/sizeof(arr[0]))

typedef unsigned int u32;
typedef unsigned long spinlock_t;

enum sw_val {
	SW_NONE = 0,
	SW_READ,
	SW_WRITE
};

struct sliding_window {
	u32 *window;	/* initialized to some given size */
	u32 size;	/* size of the sliding window in words */
	u32 *major;	/* pointer into window */
	u32 minor;	/* bit mask for the current word pointed to in window */
	u32 stat[2];	/* statistic table */
	spinlock_t lock;
};

int sw_alloc(struct sliding_window *sw, u32 size);
bool sw_advance(struct sliding_window *sw, enum sw_val val);
u32 sw_val_get(struct sliding_window *sw, enum sw_val val);
void sw_reset(struct sliding_window *sw);
void sw_free(struct sliding_window *sw);

static inline u32 sw_get_size(struct sliding_window *sw)
{
	return sw->size;
}
EXPORT_SYMBOL(sw_get_size);
#endif

