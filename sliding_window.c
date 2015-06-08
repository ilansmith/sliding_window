#include <string.h>
#include <limits.h>
#include "sliding_window.h"

/* val == 0 is reserved for SLW_NONE, which is does not have a stat counter */
#define SLW_STAT(slw, val) (slw)->stat[(val)-1]

int slw_init(struct sliding_window *slw, u32 width)
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

	slw->__window = window;
	slw->__size = size;

	slw->width = width;
	memset(slw->stat, 0, sizeof(slw->stat));

	slw->__major = slw->__window;
	slw->__offset = 0x0;

	spin_lock_init(&slw->__lock);

	return 0;
}
EXPORT_SYMBOL(slw_init);

void slw_uninit(struct sliding_window *slw)
{
	kfree(slw->__window);
	memset(slw, 0, sizeof(struct sliding_window));
}
EXPORT_SYMBOL(slw_uninit);

bool slw_advance(struct sliding_window *slw, enum slw_val val_new)
{
	unsigned long flags;
	enum slw_val val_stale;

	spin_lock_irqsave(&slw->__lock, flags);
	val_stale = (*slw->__major & ((0x3) << slw->__offset)) >> slw->__offset;

	if (val_new == val_stale)
		goto out;

	/* update counters */
	*slw->__major &= ~(0x3 << slw->__offset);
	if (val_new)
		*slw->__major |= val_new << slw->__offset;

	if (val_stale && SLW_STAT(slw, val_stale))
		SLW_STAT(slw, val_stale)--;
	if (val_new && SLW_STAT(slw, val_new) <= UINT_MAX)
		SLW_STAT(slw, val_new)++;

out:
	/* update pointers */
	slw->__offset += 2;
	if ((31 < slw->__offset) ||
		((slw->__major == slw->__window + slw->__size - 1) &&
		((slw->width * 2 - (slw->__size - 1) * 32) - 1 <
		 slw->__offset))) {
		slw->__offset = 0x0;

		if (slw->__major < slw->__window + slw->__size - 1)
			slw->__major++;
		else
			slw->__major = slw->__window;
	}
	spin_unlock_irqrestore(&slw->__lock, flags);

	return true;
}
EXPORT_SYMBOL(slw_advance);

void slw_reset(struct sliding_window *slw)
{
	unsigned long flags;

	spin_lock_irqsave(&slw->__lock, flags);

	memset(slw->__window, 0, sizeof(u32) * slw->__size);
	memset(slw->stat, 0, sizeof(slw->stat));

	slw->__major = slw->__window;
	slw->__offset = 0x0;

	spin_unlock_irqrestore(&slw->__lock, flags);
}
EXPORT_SYMBOL(slw_reset);

u32 slw_width_get(struct sliding_window *slw)
{
	return slw->width;
}
EXPORT_SYMBOL(slw_width_get);

u32 slw_val_get(struct sliding_window *slw, enum slw_val val)
{
	u32 ret;
	unsigned long flags;

	spin_lock_irqsave(&slw->__lock, flags);
	if (val == SLW_NONE) {
		ret = slw->width -
			(SLW_STAT(slw, SLW_READ) + SLW_STAT(slw, SLW_WRITE));
	} else {
		ret = SLW_STAT(slw, val);
	}
	spin_unlock_irqrestore(&slw->__lock, flags);

	return ret;
}
EXPORT_SYMBOL(slw_val_get);

