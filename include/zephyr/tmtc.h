/* tmtc.h - one line definition */

/* All Rights Reserved */

#ifndef INC_TMTC_H
#define INC_TMTC_H

/* Includes */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <zephyr/toolchain.h>

#ifdef __cplusplus
extern "C" {
#endif


/* Configurations */


/* Definitions */

#define TMTC_CMD_SECTION_NAME ".tmtc_cmd_handlers"

#ifndef TMTC_CMD_STORAGE
#define TMTC_CMD_STORAGE static
#endif

#define TMTC_CMD_SECTION_ATTR                                            \
    __attribute__((section(TMTC_CMD_SECTION_NAME)))                      \
    __attribute__((used))                                                \
    __attribute__((aligned(__alignof__(struct tmtc_cmd_handler))))

/*
 * Registration macro (matches your initializer style).
 *
 * Usage:
 * TMTC_DEFINE(my_cmds) = {
 *     { .id=1, ... },
 *     { .id=2, ... },
 * };
 */
#define TMTC_DEFINE(name) \
    TMTC_CMD_STORAGE const struct tmtc_cmd_handler name[] TMTC_CMD_SECTION_ATTR


#define TMTC_CMD_FOREACH(it)                                             \
    for (const struct tmtc_cmd_handler *(it) = __tmtc_cmd_handlers_start; \
         (it) < __tmtc_cmd_handlers_end;                                 \
         ++(it))

/* Types */

struct tmtc_args {
    size_t hdr_len;
    size_t len;
    void *data;
    bool incomplete;
};

struct tmtc_cmd_handler {
    uint16_t id;
    size_t max_data_len;
    size_t min_data_len;
    int32_t (*handler)(struct tmtc_args *rqst, struct tmtc_args *rply);
};

/* External Declarations */

extern const struct tmtc_cmd_handler __tmtc_cmd_handlers_start[];
extern const struct tmtc_cmd_handler __tmtc_cmd_handlers_end[];

extern const struct tmtc_cmd_handler *tmtc_get_cmd_handler(uint16_t cmd_id);
extern int32_t tmtc_run_cmd(uint16_t cmd_id, struct tmtc_args *rqst, struct tmtc_args *rply);
extern void *tmtc_malloc(size_t size);
extern void tmtc_free(void *ptr);


#ifdef __cplusplus
}
#endif

#endif /* INC_TMTC_H */
