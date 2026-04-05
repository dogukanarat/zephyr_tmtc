if(CONFIG_TMTC)
  zephyr_library()

  zephyr_library_sources_ifdef(CONFIG_TMTC
    src/tmtc.c
  )

  zephyr_library_sources_ifdef(CONFIG_SHELL
    src/tmtc_shell.c
  )

  zephyr_include_directories(include)

  zephyr_linker_sources_ifdef(CONFIG_TMTC SECTIONS tmtc_cmd_handlers.ld)
endif()
