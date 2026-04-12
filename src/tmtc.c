/* tmtc.c - one line definition */

/* All Rights Reserved */

/* Includes */

#include <string.h>

#include <zephyr/logging/log.h>
#include <zephyr/tmtc.h>

#include <osal/osal_memory.h>

#include <pb_decode.h>
#include <pb_encode.h>

/* Imports */


/* Definitions */

LOG_MODULE_REGISTER(tmtc, LOG_LEVEL_DBG);

/* Types */


/* Forward Declarations */


/* Variables */


/* Functions */

const struct tmtc_cmd_handler *tmtc_get_cmd_handler (uint16_t cmd_id)
{
    TMTC_CMD_FOREACH(cmd)
    {
        if (cmd->id == cmd_id)
        {
            return cmd;
        }
    }

    LOG_ERR("Command ID %u not found", cmd_id);

    return NULL;
}

int32_t tmtc_request_with_handler (
    const struct tmtc_cmd_handler *handler,
    struct tmtc_args *rqst,
    struct tmtc_args *rply)
{
    int32_t ret = 0;

    for (;;)
    {
        if (handler == NULL)
        {
            LOG_ERR("Handler is NULL");
            ret = -EINVAL;
            break;
        }

        if (!rply || !rqst)
        {
            LOG_ERR("Invalid request or reply arguments for command ID %u", handler->id);
            ret = -EINVAL;
            break;
        }

        if (handler->cb == NULL)
        {
            LOG_ERR("No handler defined for command ID %u", handler->id);
            ret = -ENOSYS;
            break;
        }

        /* Decode request protobuf into payload struct (skip if already decoded) */
        if (handler->rqst_fields && handler->rqst_size > 0 && !rqst->payload)
        {
            rqst->payload = osal_memory_alloc(handler->rqst_size);
            if (!rqst->payload)
            {
                LOG_ERR("Failed to allocate request payload for command ID %u", handler->id);
                ret = -ENOMEM;
                break;
            }
            memset(rqst->payload, 0, handler->rqst_size);

            if (rqst->data && rqst->len > 0)
            {
                pb_istream_t stream = pb_istream_from_buffer(rqst->data, rqst->len);
                if (!pb_decode(&stream, handler->rqst_fields, rqst->payload))
                {
                    LOG_ERR("Failed to decode request for command ID %u: %s",
                            handler->id, PB_GET_ERROR(&stream));
                    ret = -EBADMSG;
                    break;
                }
            }
        }

        /* Allocate reply payload struct (reuse if already allocated) */
        if (handler->rply_fields && handler->rply_size > 0)
        {
            if (!rply->payload)
            {
                rply->payload = osal_memory_alloc(handler->rply_size);
                if (!rply->payload)
                {
                    LOG_ERR("Failed to allocate reply payload for command ID %u", handler->id);
                    ret = -ENOMEM;
                    break;
                }
            }
            memset(rply->payload, 0, handler->rply_size);
        }

        /* Execute the command callback */
        ret = handler->cb(rqst, rply);
        if (ret < 0)
        {
            LOG_ERR("Handler for command ID %u returned error code %d", handler->id, ret);
            break;
        }

        /* Encode reply payload struct into protobuf */
        if (handler->rply_fields && handler->rply_size > 0 && rply->payload)
        {
            /* Determine encoded size */
            size_t encoded_size = 0;
            if (!pb_get_encoded_size(&encoded_size, handler->rply_fields, rply->payload))
            {
                LOG_ERR("Failed to get encoded size for command ID %u", handler->id);
                ret = -EINVAL;
                break;
            }

            rply->data = osal_memory_alloc(encoded_size);
            if (!rply->data)
            {
                LOG_ERR("Failed to allocate reply buffer for command ID %u", handler->id);
                ret = -ENOMEM;
                break;
            }

            pb_ostream_t stream = pb_ostream_from_buffer(rply->data, encoded_size);
            if (!pb_encode(&stream, handler->rply_fields, rply->payload))
            {
                LOG_ERR("Failed to encode reply for command ID %u: %s",
                        handler->id, PB_GET_ERROR(&stream));
                ret = -EINVAL;
                break;
            }
            rply->len = stream.bytes_written;
        }

        break;
    }

    /* Free payload buffers only when transfer is complete */
    if (!rply->incomplete)
    {
        if (rqst->payload)
        {
            osal_memory_free(rqst->payload);
            rqst->payload = NULL;
        }
        if (rply->payload)
        {
            osal_memory_free(rply->payload);
            rply->payload = NULL;
        }
    }

    return ret;
}

int32_t tmtc_request (uint16_t id, struct tmtc_args *rqst, struct tmtc_args *rply)
{
    int32_t ret = 0;

    for (;;)
    {
        if (!rply || !rqst)
        {
            LOG_ERR("Invalid request or reply arguments for command ID %u", id);
            ret = -EINVAL;
            break;
        }

        const struct tmtc_cmd_handler *handler = tmtc_get_cmd_handler(id);
        if (handler == NULL)
        {
            LOG_ERR("Command ID %u not recognized", id);
            ret = -ENOENT;
            break;
        }

        ret = tmtc_request_with_handler(handler, rqst, rply);

        break;
    }

    return ret;
}

extern int32_t tmtc_request_local (
    int16_t id,
    uint8_t *rqst_payload,
    size_t rqst_payload_len,
    uint8_t *rply_payload,
    size_t rply_payload_max_len,
    tmtc_reply_fn_t rply_fn,
    void *rply_fn_user_data)
{
    struct tmtc_args rqst = {0};
    struct tmtc_args rply = {0};

    int32_t ret = 0;
    int32_t local_ret = 0;

    for (;;)
    {
        do
        {
            rqst.payload = rqst_payload;
            rqst.len = rqst_payload_len;

            rply.payload = rply_payload;
            rply.len = 0; // Will be set by the handler
            rply.incomplete = false;

            local_ret = tmtc_request(id, &rqst, &rply);
            if (local_ret < 0)
            {
                LOG_ERR("tmtc_request failed for command ID %u with error code %d", id, local_ret);
                ret = local_ret;
                break;
            }

            if (rply_fn)
            {
                rply_fn(rply.payload, rply.len, rply_fn_user_data);
            }
        } while (rply.incomplete);

        break;
    }

    return ret;
}