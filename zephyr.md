# libplctag on Zephyr RTOS

Branch `zephyr-port` (base: libplctag v2.7.2) turns libplctag into a Zephyr
module. Target for the PoC: **Waveshare ESP32-P4-ETH**
(`waveshare_esp32p4_eth/esp32p4/hpcore`, Zephyr main, 4.5.0-rc1).

## Design

libplctag already has a POSIX platform layer (`src/platform/posix`). Zephyr
provides most of POSIX through `CONFIG_POSIX_API`, so the port reuses that
layer with a small number of `#if defined(__ZEPHYR__)` blocks. It does not
fork it.

| Area | Linux | Zephyr |
|---|---|---|
| mutex | `pthread_mutex` (recursive) | `k_mutex` (recursive by design) |
| condition var | `pthread_cond` + absolute `CLOCK_REALTIME` | `k_mutex` + `k_condvar`, relative timeout |
| `time_ms()` | `gettimeofday()` | `k_uptime_get()` (monotonic; unaffected by SNTP/PTP) |
| threads | `pthread_create(NULL attr)` | `pthread_create` with an explicit stack size, dynamic stack |
| sockets | BSD + `select()` + `socketpair()` wake | same API through `CONFIG_POSIX_API`, `CONFIG_NET_SOCKETPAIR` |
| random | `getrandom()` | `sys_csrand_get()` / `sys_rand_get()` |

Why mutexes and condition variables are native: Zephyr's pthread mutexes and
condvars come from static pools (`CONFIG_MAX_PTHREAD_MUTEX_COUNT=5`).
libplctag creates one or more per tag, session and condition variable, so the
pool runs out right at library init (`PLCTAG_ERR_MUTEX_INIT`). Kernel objects
embedded in the heap-allocated wrapper have no such limit.

Thread stacks: with `CONFIG_DYNAMIC_THREAD`, Zephyr ties the stack to the
`pthread_attr_t`. The attr is stored in `struct thread_t` and destroyed only in
`thread_destroy()`, after the join. Destroying it earlier logs
`tid ... is in use!` and leaks the stack.

## Files

    zephyr/module.yml            module definition
    zephyr/Kconfig               CONFIG_LIBPLCTAG and options
    zephyr/CMakeLists.txt        library sources (AB/CIP, PCCC, Omron, Modbus TCP)
    zephyr/zephyr_init.c         optional printk() logger
    samples/zephyr/plc_read      sample: array tag read in a loop + RTT statistics
    src/platform/posix/platform.c   __ZEPHYR__ blocks
    src/utils/random_utils.c        __ZEPHYR__ branch

## Kconfig

| Symbol | Default | Description |
|---|---|---|
| `CONFIG_LIBPLCTAG` | n | enables the library; selects POSIX_API, NET_SOCKETS, NET_TCP, NET_SOCKETPAIR, DYNAMIC_THREAD, THREAD_LOCAL_STORAGE |
| `CONFIG_LIBPLCTAG_THREAD_STACK_SIZE` | 16384 | stack of each internal thread; upstream asks for 32 KiB, 0 = upstream value |
| `CONFIG_LIBPLCTAG_DEBUG_PRINTK` | y | libplctag debug output (`debug=N`) goes to printk |

Requirement: an architecture with TLS (`ARCH_HAS_THREAD_LOCAL_STORAGE`), or
native_sim. libplctag uses `__thread`.

## Resources

Threads: tag tickler, refcount cleanup, and one handler per PLC connection.
With one PLC that is 3 threads at 16 KiB each, so `CONFIG_POSIX_THREAD_THREADS_MAX=8`
leaves headroom.

Memory:

- Thread stacks come from `CONFIG_HEAP_MEM_POOL_SIZE`. The sample uses 96–128 KiB.
- Tags and buffers come from the libc malloc arena (`CONFIG_COMMON_LIBC_MALLOC_ARENA_SIZE`).
  On the ESP32-P4 it defaults to -1 (all remaining SRAM). On native_sim the default is
  only 16 KiB, which is too small: Large Forward Open needs a 4 KiB buffer and fails
  with `Unable to increase request buffer size to 4044 bytes`.
- ESP32-P4-ETH, sample build: SRAM 241 KB / 384 KB (61 %), flash 584 KB.
  PSRAM (32 MB) is still free for the sample ring buffer.

FDs: each connection uses 3 (the TCP socket plus the socketpair).
`CONFIG_ZVFS_OPEN_MAX=24`.

## Build

West workspace (Zephyr main), libplctag as an extra module:

    west build -b waveshare_esp32p4_eth/esp32p4/hpcore \
        libplctag/samples/zephyr/plc_read \
        -- -DEXTRA_ZEPHYR_MODULES=$PWD/libplctag \
           -DCONFIG_PLC_READ_GATEWAY=\"192.168.1.10\" \
           -DCONFIG_PLC_READ_TAG_NAME=\"AnalogBuf\" \
           -DCONFIG_PLC_READ_ELEM_COUNT=800 \
           -DCONFIG_PLC_READ_PERIOD_MS=0
    west flash

Or permanently in `west.yml`:

    - name: libplctag
      url: <your fork>
      revision: zephyr-port
      path: modules/lib/libplctag

Set the board's IP address in `prj.conf` (`CONFIG_NET_CONFIG_MY_IPV4_*`) or use DHCP.

## Testing without a PLC (native_sim + ab_server)

`boards/native_sim_native_64.conf` switches the sockets to NSOS (offloaded to
the host stack), so Zephyr talks directly to `ab_server` on 127.0.0.1:

    # host: libplctag simulator
    cmake -S libplctag -B host -G Ninja && ninja -C host ab_server tag_rw2
    host/bin_dist/ab_server --plc=ControlLogix --path=1,0 \
        --tag=TestDINTArray:DINT[800] &

    # zephyr
    west build -b native_sim/native/64 libplctag/samples/zephyr/plc_read \
        -- -DEXTRA_ZEPHYR_MODULES=$PWD/libplctag \
           -DCONFIG_PLC_READ_GATEWAY=\"127.0.0.1\" -DCONFIG_PLC_READ_ELEM_COUNT=800
    ./build/zephyr/zephyr.exe

Verified result: a pattern written from Linux
(`tag_rw2 --write=11,22,33,-44,55,66,77,123456789`) reads back correctly in
Zephyr, and 1200+ reads of a 3200 B array complete with 0 errors. RTT on
native_sim is simulated time and says nothing about real performance. Take
real numbers only from the board.

## Open items

- Not yet tested on the ESP32-P4-ETH hardware against a real Logix (only builds).
- Reconnect after a cable pull or PLC restart is not tested on Zephyr
  (`test_reconnect*` from `src/tests`).
- Modbus RTU (serial) is not part of the POSIX layer, and therefore not part of the port.
- DNS: `gateway=` with a hostname needs `CONFIG_DNS_RESOLVER=y`; an IP literal does not.
- Next step for the PoC: IIO over USB (libplctag thread → ring buffer in PSRAM → IIO buffer).
