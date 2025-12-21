/* tmtc.c - one line definition */

/* All Rights Reserved */

/* Includes */

#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/tmtc.h>

/* Imports */


/* Definitions */

LOG_MODULE_REGISTER(tmtc, LOG_LEVEL_DBG);

/* Types */


/* Forward Declarations */


/* Variables */


/* Functions */

const struct tmtc_cmd_handler *tmtc_get_cmd_handler(uint16_t cmd_id)
{
    TMTC_CMD_FOREACH(cmd) {
        if (cmd->id == cmd_id) {
            return cmd;
        }
    }

    LOG_ERR("Command ID %u not found", cmd_id);

    return NULL;
}

int32_t tmtc_run_cmd(uint16_t cmd_id, struct tmtc_args *rqst, struct tmtc_args *rply)
{
    if (!rply || !rqst) {
        LOG_ERR("Invalid request or reply arguments for command ID %u", cmd_id);
        return -EINVAL;
    }

    const struct tmtc_cmd_handler *cmd = tmtc_get_cmd_handler(cmd_id);
    if (cmd == NULL) {
        LOG_ERR("Command ID %u not recognized", cmd_id);
        return -ENOENT;
    }

    if (cmd->handler == NULL) {
        LOG_ERR("No handler defined for command ID %u", cmd_id);
        return -ENOSYS;
    }

    /* Validate request data length */
    if (rqst->len < cmd->min_data_len || rqst->len > cmd->max_data_len) {
        LOG_ERR("Invalid data length for command ID %u: received %u, expected [%u, %u]",
                cmd_id, rqst->len, cmd->min_data_len, cmd->max_data_len);
        return -EMSGSIZE;
    }

    /* Call the command handler */
    return cmd->handler(rqst, rply);
}

void *tmtc_malloc(size_t size)
{
    void *ptr = k_malloc(size);
    if (ptr == NULL) {
        LOG_ERR("Memory allocation of size %u failed", size);
    }
    return ptr;
}

void tmtc_free(void *ptr)
{
    k_free(ptr);
}
