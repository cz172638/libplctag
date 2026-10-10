/* SPDX-License-Identifier: MPL-2.0 OR LGPL-2.0-or-later */
/*
 * libplctag on Zephyr: read one DINT/REAL array tag in a loop and print
 * round-trip statistics.  This is the same access pattern as on Linux:
 * one array tag, Large Forward Open negotiated automatically by libplctag.
 */

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/version.h>

#include <libplctag.h>

#define TAG_ATTRS                                                                  \
	"protocol=ab-eip&gateway=" CONFIG_PLC_READ_GATEWAY                         \
	"&path=" CONFIG_PLC_READ_PATH "&plc=" CONFIG_PLC_READ_PLC_TYPE             \
	"&elem_size=4&elem_count=" STRINGIFY(CONFIG_PLC_READ_ELEM_COUNT)            \
	"&name=" CONFIG_PLC_READ_TAG_NAME

#define STATS_EVERY 50

int main(void)
{
	int32_t tag;
	int rc;
	uint32_t n = 0, errors = 0;
	int64_t t_min = INT64_MAX, t_max = 0, t_sum = 0;

	printk("libplctag %d.%d.%d on Zephyr %s, board %s\n",
	       plc_tag_get_int_attribute(0, "version_major", 0),
	       plc_tag_get_int_attribute(0, "version_minor", 0),
	       plc_tag_get_int_attribute(0, "version_patch", 0),
	       KERNEL_VERSION_STRING, CONFIG_BOARD_TARGET);

	plc_tag_set_debug_level(CONFIG_PLC_READ_DEBUG);

	const char *attrs = sizeof(CONFIG_PLC_READ_ATTRS) > 1 ? CONFIG_PLC_READ_ATTRS : TAG_ATTRS;
	bool modbus = strstr(attrs, "modbus") != NULL;

	printk("creating tag: %s\n", attrs);

	tag = plc_tag_create(attrs, CONFIG_PLC_READ_TIMEOUT_MS);
	if (tag < 0) {
		printk("plc_tag_create failed: %s\n", plc_tag_decode_error(tag));
		return 0;
	}

	printk("tag %d created, size %d bytes\n", tag, plc_tag_get_size(tag));

	while (1) {
		int64_t t0 = k_uptime_ticks();

		rc = plc_tag_read(tag, CONFIG_PLC_READ_TIMEOUT_MS);

		int64_t dt_us = k_ticks_to_us_near64(k_uptime_ticks() - t0);

		if (rc != PLCTAG_STATUS_OK) {
			errors++;
			printk("read #%u failed: %s\n", n, plc_tag_decode_error(rc));
		} else {
			t_min = MIN(t_min, dt_us);
			t_max = MAX(t_max, dt_us);
			t_sum += dt_us;
		}

		n++;

		if (n == 1 && rc == PLCTAG_STATUS_OK) {
			int esz = modbus ? 2 : 4;
			int cnt = MIN(plc_tag_get_size(tag) / esz, 8);

			printk("first read:");
			for (int i = 0; i < cnt; i++) {
				printk(" %" PRId32, modbus ? (int32_t)plc_tag_get_uint16(tag, i * 2)
							   : plc_tag_get_int32(tag, i * 4));
			}
			printk("%s\n", plc_tag_get_size(tag) / esz > 8 ? " ..." : "");
		}

		if ((n % STATS_EVERY) == 0) {
			uint32_t ok = n - errors;

			printk("reads %u errors %u  rtt us: min %" PRId64 " avg %" PRId64
			       " max %" PRId64 "\n",
			       n, errors, ok ? t_min : 0, ok ? t_sum / ok : 0, t_max);
		}

		if (CONFIG_PLC_READ_PERIOD_MS > 0) {
			k_msleep(CONFIG_PLC_READ_PERIOD_MS);
		}
	}

	return 0;
}
