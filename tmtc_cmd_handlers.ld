/*
 * Inserted inside the existing rodata output section via
 * zephyr_linker_sources_ifdef(CONFIG_TMTC RODATA ...). Do NOT wrap this in
 * a standalone `.tmtc_cmd_handlers : { ... }` output section — that would
 * create an orphan LOAD segment with an invalid virtual address (observed
 * on ESP32 as load:0x000NNNNN,len:168 followed by RTCWDT_RTC_RESET at
 * boot), because the handlers would no longer inherit the DROM VMA of the
 * surrounding rodata.
 */
. = ALIGN(4);
__tmtc_cmd_handlers_start = .;
KEEP(*(.tmtc_cmd_handlers))
KEEP(*(.tmtc_cmd_handlers.*))
__tmtc_cmd_handlers_end = .;
