#ifndef _SLIDING_WINDOW_COMPAT_H_
#define _SLIDING_WINDOW_COMPAT_H_

#include <stdlib.h>
#include <stdio.h>

#define EXPORT_SYMBOL(sym)

#define EINVAL 2
#define ENOMEM 3

#define bool int
#define true 1
#define false 0

#define spin_lock_init(lock) \
	do { \
	} while (0)
#define spin_lock_irqsave(lock, flags) \
	do { \
		if (0) \
			flags = 0; \
	} while (0)
#define spin_unlock_irqrestore(lock, flags) \
	do { \
		if (0) \
			*lock = (spinlock_t)flags; \
	} while (0)

#define kmalloc(size, gfp) calloc(1, size)
#define kfree(ptr) free(ptr)

#define pr_err(str, ...) printf(str, ##__VA_ARGS__)

#endif
