# libplctag as a Zephyr module

Build libplctag into a Zephyr application with `CONFIG_LIBPLCTAG=y`.
Full notes (design, memory, testing): see `zephyr.md` on the `gh-pages` branch.

Quick start:

    west build -b waveshare_esp32p4_eth/esp32p4/hpcore samples/zephyr/plc_read \
        -- -DEXTRA_ZEPHYR_MODULES=$PWD \
           -DCONFIG_PLC_READ_GATEWAY=\"192.168.1.10\"

Host test against the bundled `ab_server` simulator:

    west build -b native_sim/native/64 samples/zephyr/plc_read \
        -- -DEXTRA_ZEPHYR_MODULES=$PWD -DCONFIG_PLC_READ_GATEWAY=\"127.0.0.1\"
