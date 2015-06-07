#include <string.h>
#include <limits.h>
#include "sliding_window.h"

int sw_alloc(struct sliding_window *sw, u32 size)
{
	u32 *window;

	if (!size) {
		pr_err("sliding_window: invalid window size\n");
		return -EINVAL;
	}

	window = kmalloc(sizeof(u32) * size, GFP_KERNEL);
	if (!window)
		return -ENOMEM;

	sw->window = window;
	sw->size = size;

	memset(sw->stat, 0, sizeof(sw->stat));

	sw->major = sw->window;
	sw->minor = 0x1;

	spin_lock_init(&sw->lock);

	return 0;
}
EXPORT_SYMBOL(sw_alloc);

bool sw_advance(struct sliding_window *sw, enum sw_val val)
{
	unsigned long flags;
	enum sw_val stale;

	spin_lock_irqsave(&sw->lock, flags);
	stale = *sw->major & sw->minor ? SW_WRITE : SW_READ;

	if (val == SW_READ)
		*sw->major &= ~(sw->minor);
	else /* val == SW_WRITE */
		*sw->major |= sw->minor;

	if (sw->stat[stale])
		sw->stat[stale]--;
	if (sw->stat[val] <= UINT_MAX)
		sw->stat[val]++;

	sw->minor <<= 0x1;
	if (!sw->minor) {
		sw->minor = 0x1;

		if (sw->major < sw->window + sw->size - 1)
			sw->major++;
		else
			sw->major = sw->window;
	}
	spin_unlock_irqrestore(&sw->lock, flags);

	return true;
}
EXPORT_SYMBOL(sw_advance);

u32 sw_val_get(struct sliding_window *sw, enum sw_val val)
{
	u32 ret;
	unsigned long flags;

	spin_lock_irqsave(&sw->lock, flags);
	ret = sw->stat[val];
	spin_unlock_irqrestore(&sw->lock, flags);

	return ret;
}
EXPORT_SYMBOL(sw_val_get);

void sw_reset(struct sliding_window *sw)
{
	unsigned long flags;

	spin_lock_irqsave(&sw->lock, flags);

	memset(sw->window, 0, sizeof(u32) * sw->size);
	memset(sw->stat, 0, sizeof(sw->stat));

	sw->major = sw->window;
	sw->minor = 0x1;

	spin_unlock_irqrestore(&sw->lock, flags);
}
EXPORT_SYMBOL(sw_reset);

void sw_free(struct sliding_window *sw)
{
	kfree(sw->window);
	memset(sw, 0, sizeof(struct sliding_window));
}
EXPORT_SYMBOL(sw_free);

