# SPDX-License-Identifier: MPL-2.0 OR LGPL-2.0-or-later
#
# Zephyr module build of libplctag.  Generated files (version.h,
# debug_generated.h, debug_module_names.c) are tracked in git upstream,
# so no code generation step is needed here.

if(CONFIG_LIBPLCTAG)

set(LPT_SRC ${CMAKE_CURRENT_LIST_DIR}/../src)
set(LPT_LIB ${LPT_SRC}/libplctag)

zephyr_library_named(plctag)

zephyr_library_sources(
  ${LPT_LIB}/lib/init.c
  ${LPT_LIB}/lib/lib.c
  ${LPT_LIB}/lib/version.c

  ${LPT_LIB}/protocols/ab/ab_common.c
  ${LPT_LIB}/protocols/ab/cip.c
  ${LPT_LIB}/protocols/ab/connection_tag.c
  ${LPT_LIB}/protocols/ab/eip_cip.c
  ${LPT_LIB}/protocols/ab/eip_cip_special.c
  ${LPT_LIB}/protocols/ab/eip_lgx_pccc.c
  ${LPT_LIB}/protocols/ab/eip_plc5_dhp.c
  ${LPT_LIB}/protocols/ab/eip_plc5_pccc.c
  ${LPT_LIB}/protocols/ab/eip_slc_dhp.c
  ${LPT_LIB}/protocols/ab/eip_slc_pccc.c
  ${LPT_LIB}/protocols/ab/error_codes.c
  ${LPT_LIB}/protocols/ab/pccc.c
  ${LPT_LIB}/protocols/ab/session.c

  ${LPT_LIB}/protocols/system/system.c

  ${LPT_SRC}/utils/atomic_utils.c
  ${LPT_SRC}/utils/attr.c
  ${LPT_SRC}/utils/debug.c
  ${LPT_SRC}/utils/debug_module_names.c
  ${LPT_SRC}/utils/hash.c
  ${LPT_SRC}/utils/hashtable.c
  ${LPT_SRC}/utils/random_utils.c
  ${LPT_SRC}/utils/rc.c
  ${LPT_SRC}/utils/vector.c

  ${LPT_SRC}/platform/posix/platform.c
)

zephyr_library_sources_ifdef(CONFIG_LIBPLCTAG_OMRON
  ${LPT_LIB}/protocols/omron/cip.c
  ${LPT_LIB}/protocols/omron/conn.c
  ${LPT_LIB}/protocols/omron/omron_common.c
  ${LPT_LIB}/protocols/omron/omron_connection_tag.c
  ${LPT_LIB}/protocols/omron/omron_raw_tag.c
  ${LPT_LIB}/protocols/omron/omron_standard_tag.c
)

zephyr_library_sources_ifdef(CONFIG_LIBPLCTAG_MODBUS
  ${LPT_LIB}/protocols/mb/modbus.c
)

# init.c, lib.c and ab_common.c reference the optional modules directly:
# link no-op stubs for the disabled ones instead of patching the core.
if(NOT CONFIG_LIBPLCTAG_OMRON OR NOT CONFIG_LIBPLCTAG_MODBUS)
  zephyr_library_sources(${CMAKE_CURRENT_LIST_DIR}/proto_stubs.c)
endif()

zephyr_library_sources_ifdef(CONFIG_LIBPLCTAG_DEBUG_PRINTK
  ${CMAKE_CURRENT_LIST_DIR}/zephyr_init.c
)

# Applications: #include <libplctag.h>
zephyr_include_directories(${LPT_LIB}/lib)

zephyr_library_include_directories(
  ${LPT_SRC}
  ${LPT_SRC}/platform/posix
)

zephyr_library_compile_definitions(LIBPLCTAG_STATIC)

# Upstream builds with -std=c11 and is not -Wshadow/-Wundef clean.
zephyr_library_compile_options(
  -Wno-shadow
  -Wno-undef
  -Wno-unused-parameter
  -Wno-enum-compare
  -Wno-double-promotion
)

endif()
