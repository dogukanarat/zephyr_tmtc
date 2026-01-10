/* tmtc_shell.c - one line definition */

/* All Rights Reserved */

/* Includes */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>
#include <zephyr/tmtc.h>

/* Imports */


/* Definitions */

LOG_MODULE_DECLARE(tmtc);

/* Types */


/* Forward Declarations */

static int tmtc_shell_list(const struct shell *shell, size_t argc, char **argv);
static int tmtc_shell_run(const struct shell *shell, size_t argc, char **argv);

/* Variables */

SHELL_STATIC_SUBCMD_SET_CREATE(sub_tmtc_cmds,
    SHELL_CMD(
        list,
        NULL,
        "List all registered commands\nUsage: tmtc list",
        tmtc_shell_list
    ),
    SHELL_CMD(
        run,
        NULL,
        "Run a command\nUsage: tmtc run <cmd_id> [data_hex]",
        tmtc_shell_run
    ),
    SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(tmtc, &sub_tmtc_cmds, "Base command for TMTC operations", NULL);

/* Functions */

static int tmtc_shell_list(const struct shell *shell, size_t argc, char **argv)
{
    shell_print(shell, "Registered TMTC Commands:");
    TMTC_CMD_FOREACH(cmd) {
        shell_print(shell, "  ID: %u, Callback: %p, Min Data Len: %u, Max Data Len: %u, Description: %s",
                    cmd->id,
                    (void *)cmd->handler,
                    cmd->min_data_len,
                    cmd->max_data_len,
                    "(no description)");
    }

    return 0;
}

static int tmtc_shell_run(const struct shell *shell, size_t argc, char **argv)
{
    if (argc < 2) {
        shell_print(shell, "Error: Command ID is required.\nUsage: tmtc run <cmd_id> [data_hex]");
        return -EINVAL;
    }

    uint16_t cmd_id = strtoul(argv[1], NULL, 0);
    const struct tmtc_cmd_handler *handler = tmtc_get_cmd_handler(cmd_id);
    if (handler == NULL) {
        shell_print(shell, "Command ID %u not found.", cmd_id);
        return -ENOEXEC;
    }

    struct tmtc_args rqst = {0};
    struct tmtc_args rply = {0};

    if (argc >= 3) {
        /* Convert hex string to binary data */
        size_t data_len = strlen(argv[2]) / 2;
        if (data_len > handler->max_data_len) {
            shell_print(shell, "Error: Data length exceeds maximum of %u bytes.", handler->max_data_len);
            return -EINVAL;
        }

        rqst.data = malloc(data_len);
        if (rqst.data == NULL) {
            return -ENOMEM;
        }

        for (size_t i = 0; i < data_len; i++) {
            sscanf(&argv[2][i * 2], "%2hhx", &((uint8_t *)rqst.data)[i]);
        }
        rqst.len = data_len;
    }

    rply.ops.malloc = (uint8_t *(*)(size_t))malloc;

    int32_t ret = tmtc_run_handler(handler, &rqst, &rply);

    shell_print(shell, "Command ID %d executed with result: 0x%08X", cmd_id, ret);
    shell_print(shell, "Reply Data (%u bytes):", rply.len);
    if (rply.data && rply.len > 0) {
        shell_hexdump(shell, (uint8_t *)rply.data, rply.len);
    } else {
        shell_print(shell, "<no data>");
    }

    if (rqst.data) {
        free(rqst.data);
    }
    if (rply.data) {
        free(rply.data);
    }

    return ret;
}
