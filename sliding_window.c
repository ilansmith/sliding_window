#include <string.h>
#include <limits.h>
#include "sliding_window.h"

/* val == 0 is reserved for SW_NONE, which is does not have a stat counter */
#define SW_STAT(sw, val) (sw)->stat[(val)-1]

int sw_init(struct sliding_window *sw, u32 width)
{
	u32 *window, size;

	if (!width) {
		pr_err("sliding_window: invalid window width\n");
		return -EINVAL;
	}

	size = ((2 * width + 32 - 1) / 32); /* size in words */
	window = (u32*)kmalloc(size * sizeof(u32), GFP_KERNEL);
	if (!window)
		return -ENOMEM;

	sw->__window = window;
	sw->__size = size;

	sw->width = width;
	memset(sw->stat, 0, sizeof(sw->stat));

	sw->__major = sw->__window;
	sw->__offset = 0x0;

	spin_lock_init(&sw->lock);

	return 0;
}
EXPORT_SYMBOL(sw_init);

void sw_uninit(struct sliding_window *sw)
{
	kfree(sw->__window);
	memset(sw, 0, sizeof(struct sliding_window));
}
EXPORT_SYMBOL(sw_uninit);

bool sw_advance(struct sliding_window *sw, enum sw_val val_new)
{
	unsigned long flags;
	enum sw_val val_stale;

	spin_lock_irqsave(&sw->lock, flags);
	val_stale = (*sw->__major & ((0x3) << sw->__offset)) >> sw->__offset;

	if (val_new == val_stale)
		goto out;

	/* update counters */
	*sw->__major &= ~(0x3 << sw->__offset);
	if (val_new)
		*sw->__major |= val_new << sw->__offset;

	if (val_stale && SW_STAT(sw, val_stale))
		SW_STAT(sw, val_stale)--;
	if (val_new && SW_STAT(sw, val_new) <= UINT_MAX)
		SW_STAT(sw, val_new)++;

out:
	/* update pointers */
	sw->__offset += 2;
	if ((31 < sw->__offset) ||
		((sw->__major == sw->__window + sw->__size - 1) &&
		((sw->width * 2 - (sw->__size - 1) * 32) - 1 < sw->__offset))) {
		sw->__offset = 0x0;

		if (sw->__major < sw->__window + sw->__size - 1)
			sw->__major++;
		else
			sw->__major = sw->__window;
	}
	spin_unlock_irqrestore(&sw->lock, flags);

	return true;
}
EXPORT_SYMBOL(sw_advance);

void sw_reset(struct sliding_window *sw)
{
	unsigned long flags;

	spin_lock_irqsave(&sw->lock, flags);

	memset(sw->__window, 0, sizeof(u32) * sw->__size);
	memset(sw->stat, 0, sizeof(sw->stat));

	sw->__major = sw->__window;
	sw->__offset = 0x0;

	spin_unlock_irqrestore(&sw->lock, flags);
}
EXPORT_SYMBOL(sw_reset);

u32 sw_width_get(struct sliding_window *sw)
{
	return sw->width;
}
EXPORT_SYMBOL(sw_width_get);

u32 sw_val_get(struct sliding_window *sw, enum sw_val val)
{
	u32 ret;
	unsigned long flags;

	spin_lock_irqsave(&sw->lock, flags);
	if (val == SW_NONE) {
		ret = sw->width -
			(SW_STAT(sw, SW_READ) + SW_STAT(sw, SW_WRITE));
	} else {
		ret = SW_STAT(sw, val);
	}
	spin_unlock_irqrestore(&sw->lock, flags);

	return ret;
}
EXPORT_SYMBOL(sw_val_get);

