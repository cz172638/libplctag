/* SPDX-License-Identifier: MPL-2.0 OR LGPL-2.0-or-later */
/*
 * Zephyr glue: send libplctag debug output to printk() instead of stderr.
 */

#include <stdint.h>
#include <zephyr/init.h>
#include <zephyr/sys/printk.h>

#include <libplctag.h>

static void libplctag_printk_logger(int32_t tag_id, int debug_level, const char *message)
{
	ARG_UNUSED(tag_id);
	ARG_UNUSED(debug_level);

	/* libplctag lines already end with '\n' */
	printk("%s", message);
}

static int libplctag_zephyr_init(void)
{
	(void)plc_tag_register_logger(libplctag_printk_logger);
	return 0;
}

SYS_INIT(libplctag_zephyr_init, APPLICATION, 0);
