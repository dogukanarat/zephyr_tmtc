if(CONFIG_TMTC)
  zephyr_library()

  zephyr_library_sources_ifdef(CONFIG_TMTC
    src/tmtc.c
  )

  zephyr_library_sources_ifdef(CONFIG_SHELL
    src/tmtc_shell.c
  )

  zephyr_include_directories(include)

  # Splice the iterable handler symbols into the existing rodata output
  # section (the handlers are `const` data). Using the `SECTIONS` slot
  # instead would create a standalone orphan section with an invalid
  # virtual address, causing ESP32 to bootloop at
  # load:0x000NNNNN,len:<size> → RTCWDT_RTC_RESET.
  zephyr_linker_sources_ifdef(CONFIG_TMTC RODATA tmtc_cmd_handlers.ld)
endif()
