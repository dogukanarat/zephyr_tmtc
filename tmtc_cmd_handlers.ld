.tmtc_cmd_handlers :
{
    . = ALIGN(4);
    __tmtc_cmd_handlers_start = .;
    KEEP(*(.tmtc_cmd_handlers))
    KEEP(*(.tmtc_cmd_handlers.*))
    __tmtc_cmd_handlers_end = .;
}
