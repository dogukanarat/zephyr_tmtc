if(CONFIG_TMTC)
  zephyr_library()

  zephyr_library_sources_ifdef(CONFIG_TMTC
    lib/tmtc.c
  )

  zephyr_library_sources_ifdef(CONFIG_SHELL
    lib/tmtc_shell.c
  )

  zephyr_include_directories(include)

  zephyr_linker_sources_ifdef(CONFIG_TMTC SECTIONS tmtc_cmd_handlers.ld)
endif()
