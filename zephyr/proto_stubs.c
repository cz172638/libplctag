/* SPDX-License-Identifier: MPL-2.0 OR LGPL-2.0-or-later */
/*
 * Stubs for protocol modules disabled in Kconfig.  The library core calls
 * <proto>_init()/<proto>_teardown() unconditionally and dispatches tag
 * creation by attribute; a NULL tag makes plc_tag_create() return
 * PLCTAG_ERR_CREATE.
 */

#include <libplctag/lib/libplctag.h>
#include <libplctag/lib/tag.h>
#include <utils/attr.h>
#include <utils/debug.h>

#if !defined(CONFIG_LIBPLCTAG_MODBUS)
#include <libplctag/protocols/mb/modbus.h>

int mb_init(void) { return PLCTAG_STATUS_OK; }

void mb_teardown(void) {}

plc_tag_p mb_tag_create(attr attribs, void (*tag_callback_func)(int32_t tag_id, int event, int status, void *userdata),
                        void *userdata, plc_tag_p src_tag) {
    (void)attribs;
    (void)tag_callback_func;
    (void)userdata;
    (void)src_tag;
    pdebug(DEBUG_MODULE_LIB, DEBUG_WARN, 0, "Modbus support not built in (CONFIG_LIBPLCTAG_MODBUS=n)!");
    return NULL;
}
#endif

#if !defined(CONFIG_LIBPLCTAG_OMRON)
#include <libplctag/protocols/omron/omron.h>

int omron_init(void) { return PLCTAG_STATUS_OK; }

void omron_teardown(void) {}

plc_tag_p omron_tag_create(attr attribs, void (*tag_callback_func)(int32_t tag_id, int event, int status, void *userdata),
                           void *userdata, plc_tag_p src_tag) {
    (void)attribs;
    (void)tag_callback_func;
    (void)userdata;
    (void)src_tag;
    pdebug(DEBUG_MODULE_LIB, DEBUG_WARN, 0, "Omron support not built in (CONFIG_LIBPLCTAG_OMRON=n)!");
    return NULL;
}
#endif
