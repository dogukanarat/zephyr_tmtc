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
    TMTC_CMD_FOREACH(cmd)
    {
        if (cmd->id == cmd_id) {
            return cmd;
        }
    }

    LOG_ERR("Command ID %u not found", cmd_id);

    return NULL;
}

int32_t tmtc_run_handler(
    const struct tmtc_cmd_handler *handler, struct tmtc_args *rqst, struct tmtc_args *rply)
{
    if (handler == NULL) {
        LOG_ERR("Handler is NULL");
        return -EINVAL;
    }

    if (!rply || !rqst) {
        LOG_ERR("Invalid request or reply arguments for command ID %u", handler->id);
        return -EINVAL;
    }

    if (handler->handler == NULL) {
        LOG_ERR("No handler defined for command ID %u", handler->id);
        return -ENOSYS;
    }

    /* Validate request data length */
    if (rqst->len < handler->min_data_len || rqst->len > handler->max_data_len) {
        LOG_ERR(
            "Invalid data length for command ID %u: received %u, expected [%u, %u]",
            handler->id,
            rqst->len,
            handler->min_data_len,
            handler->max_data_len);
        return -EMSGSIZE;
    }

    /* Call the command handler */
    return handler->handler(rqst, rply);
}

int32_t tmtc_run_id(uint16_t id, struct tmtc_args *rqst, struct tmtc_args *rply)
{
    if (!rply || !rqst) {
        LOG_ERR("Invalid request or reply arguments for command ID %u", id);
        return -EINVAL;
    }

    const struct tmtc_cmd_handler *handler = tmtc_get_cmd_handler(id);
    if (handler == NULL) {
        LOG_ERR("Command ID %u not recognized", id);
        return -ENOENT;
    }

    return tmtc_run_handler(handler, rqst, rply);
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
