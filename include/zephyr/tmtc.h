/* tmtc.h - one line definition */

/* All Rights Reserved */

#ifndef INC_TMTC_H
#define INC_TMTC_H

/* Includes */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/toolchain.h>

#ifdef __cplusplus
extern "C"
{
#endif


/* Configurations */


/* Definitions */

/// @brief Name of the section where TMTC command handlers will be placed in the binary. This allows for easy identification and iteration over the command handlers at runtime.
#define TMTC_CMD_SECTION_NAME ".tmtc_cmd_handlers"

/// @brief Macro to specify the storage class for TMTC command handler definitions. By default, it is set to `static`, which means that the command handlers will have internal linkage and will not be visible outside of the translation unit in which they are defined. This can help to prevent naming conflicts and reduce the risk of unintended interactions between different parts of the codebase. However, this macro can be overridden if a different storage class is desired for the command handlers.
#ifndef TMTC_CMD_STORAGE
#define TMTC_CMD_STORAGE static
#endif

/// @brief Attribute macro to place TMTC command handler definitions in a specific section of the binary. This ensures that all command handlers are grouped together and can be easily iterated over at runtime. The attribute also marks the handlers as used to prevent them from being optimized away by the linker, and ensures proper alignment based on the structure of the command handler.
#define TMTC_CMD_SECTION_ATTR                                                                      \
    __attribute__((section(TMTC_CMD_SECTION_NAME))) __attribute__((used))                          \
    __attribute__((aligned(__alignof__(struct tmtc_cmd_handler))))

/// @brief Macro to define a TMTC command handler. This macro should be used to declare and initialize an array of `struct tmtc_cmd_handler` for each command handler implementation. The `name` parameter specifies the name of the array variable that will hold the command handlers.
#define TMTC_DEFINE(name)                                                                          \
    TMTC_CMD_STORAGE const struct tmtc_cmd_handler name[] TMTC_CMD_SECTION_ATTR

/// @brief Macro to iterate over all registered TMTC command handlers. The loop variable `it` will point to each handler in turn.
#define TMTC_CMD_FOREACH(it)                                                                       \
    for (const struct tmtc_cmd_handler *(it) = __tmtc_cmd_handlers_start;                          \
         (it) < __tmtc_cmd_handlers_end;                                                           \
         ++(it))

/* Types */

/// @brief Structure representing the arguments for a TMTC command, including header and payload information.
struct tmtc_args
{
    size_t hdr_len;   /// Length of the header data (if any)
    size_t len;       /// Length of the total data (header + payload)
    uint8_t *data;    /// Pointer to raw buffer containing header and payload (protobuf)
    uint8_t *payload; /// Pointer to payload buffer (decoded from protobuf, if applicable)
    bool incomplete;  /// Indicates if the data is incomplete
};

/// @brief Forward declaration of the command handler function type
typedef int32_t (*tmtc_callback_fn_t)(struct tmtc_args *rqst, struct tmtc_args *rply);

typedef int32_t (*tmtc_reply_fn_t)(void *data, size_t size, void *user_data);

/// @brief Structure representing a TMTC command handler, including command ID, data length constraints, and the handler function.
struct tmtc_cmd_handler
{
    uint16_t id;           /// Command ID
    void *rqst_fields;     /// Potobuf field descriptions for request decoding (if applicable)
    size_t rqst_size;      /// Maximum allowed data length for this command
    void *rply_fields;     /// Protobuf field descriptions for reply encoding (if applicable)
    size_t rply_size;      /// Minimum required data length for this command
    tmtc_callback_fn_t cb; /// Command handler function
};

/* External Declarations */

/// @brief Extern symbols marking the start and end of the TMTC command handler section in the binary. These symbols are used to iterate over the registered command handlers at runtime.
extern const struct tmtc_cmd_handler __tmtc_cmd_handlers_start[];
extern const struct tmtc_cmd_handler __tmtc_cmd_handlers_end[];

/// @brief Function prototypes for TMTC command handling and memory allocation. These functions provide the core functionality for retrieving command handlers, executing commands by ID or handler, and allocating memory for command replies.
/// @param cmd_id The ID of the command to retrieve the handler for.
/// @return Returns a pointer to the command handler structure corresponding to the given command ID, or NULL if no handler is found for that ID.
extern const struct tmtc_cmd_handler *tmtc_get_cmd_handler (uint16_t cmd_id);

/// @brief Function prototypes for TMTC command handling and memory allocation. These functions provide the core functionality for retrieving command handlers, executing commands by ID or handler, and allocating memory for command replies.
/// @param handler Pointer to the command handler to execute.
/// @param rqst Pointer to the request arguments for the command.
/// @param rply Pointer to the reply arguments for the command.
/// @return Returns the result of executing the command, or a negative error code if execution fails
extern int32_t tmtc_request_with_handler (
    const struct tmtc_cmd_handler *handler,
    struct tmtc_args *rqst,
    struct tmtc_args *rply);

/// @brief Function prototypes for TMTC command handling and memory allocation. These functions provide the core functionality for retrieving command handlers, executing commands by ID or handler, and allocating memory for command replies.
/// @param id The ID of the command to execute.
/// @param rqst Pointer to the request arguments for the command.
/// @param rply Pointer to the reply arguments for the command.
/// @return Returns the result of executing the command, or a negative error code if execution fails.
extern int32_t tmtc_request (uint16_t id, struct tmtc_args *rqst, struct tmtc_args *rply);

/// @brief Function prototypes for TMTC command handling and memory allocation. These functions provide the core functionality for retrieving command handlers, executing commands by ID or handler, and allocating memory for command replies.
/// @param id The ID of the command to execute.
/// @param rqst_payload Pointer to the request payload data for the command.
/// @param rqst_payload_len Length of the request payload data in bytes.
/// @param rply_payload Pointer to the buffer where the reply payload should be stored.
/// @param rply_payload_max_len Maximum length of the reply payload buffer in bytes.
/// @param rply_fn Callback function to be called with the reply data once the command has been processed. The callback receives ownership of the data pointer and must call `free()` when done.
/// @param rply_fn_user_data Opaque pointer that will be passed to the reply callback function when it is called.
/// @return Returns the result of executing the command, or a negative error code if execution fails.
extern int32_t tmtc_request_local (
    int16_t id,
    uint8_t *rqst_payload,
    size_t rqst_payload_len,
    uint8_t *rply_payload,
    size_t rply_payload_max_len,
    tmtc_reply_fn_t rply_fn,
    void *rply_fn_user_data);

#ifdef __cplusplus
}
#endif

#endif /* INC_TMTC_H */
