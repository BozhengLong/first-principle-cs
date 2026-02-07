/**
 * types.h - Core type definitions for distributed key-value store
 *
 * Defines status codes, enumerations, and fundamental types used
 * throughout the dist-kv system.
 */

#ifndef DIST_KV_TYPES_H
#define DIST_KV_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Status codes for operations */
typedef enum {
    DKV_OK = 0,              /* Operation succeeded */
    DKV_ERR_NOMEM,           /* Memory allocation failed */
    DKV_ERR_INVALID,         /* Invalid argument */
    DKV_ERR_NOT_FOUND,       /* Key not found */
    DKV_ERR_EXISTS,          /* Key already exists */
    DKV_ERR_TIMEOUT,         /* Operation timed out */
    DKV_ERR_NOT_LEADER,      /* Node is not the leader */
    DKV_ERR_NO_QUORUM,       /* Cannot reach quorum */
    DKV_ERR_PARTITION,       /* Network partition detected */
    DKV_ERR_READONLY,        /* Node is in read-only mode */
    DKV_ERR_SHUTDOWN,        /* Node is shutting down */
    DKV_ERR_IO,              /* I/O error */
    DKV_ERR_INTERNAL,        /* Internal error */
} dkv_status_t;

/* Node states */
typedef enum {
    NODE_STATE_INIT = 0,     /* Initial state */
    NODE_STATE_STARTING,     /* Node is starting up */
    NODE_STATE_RUNNING,      /* Node is running normally */
    NODE_STATE_STOPPING,     /* Node is shutting down */
    NODE_STATE_STOPPED,      /* Node has stopped */
    NODE_STATE_FAILED,       /* Node has failed */
} node_state_t;

/* Partition states */
typedef enum {
    PARTITION_STATE_INIT = 0,
    PARTITION_STATE_ACTIVE,
    PARTITION_STATE_MIGRATING,
    PARTITION_STATE_READONLY,
    PARTITION_STATE_OFFLINE,
} partition_state_t;

/* Consistency levels for reads */
typedef enum {
    CONSISTENCY_LINEARIZABLE,  /* Strong consistency via Raft */
    CONSISTENCY_EVENTUAL,      /* Read from local replica */
} consistency_level_t;

/* Node identifier */
typedef uint64_t node_id_t;

/* Partition identifier */
typedef uint32_t partition_id_t;

/* Key-value pair */
typedef struct {
    const char *key;
    size_t key_len;
    const void *value;
    size_t value_len;
} kv_pair_t;

/* Node address */
typedef struct {
    char host[256];
    uint16_t port;
} node_addr_t;

/* Convert status code to string */
static inline const char *dkv_status_str(dkv_status_t status) {
    switch (status) {
        case DKV_OK:            return "OK";
        case DKV_ERR_NOMEM:     return "Out of memory";
        case DKV_ERR_INVALID:   return "Invalid argument";
        case DKV_ERR_NOT_FOUND: return "Not found";
        case DKV_ERR_EXISTS:    return "Already exists";
        case DKV_ERR_TIMEOUT:   return "Timeout";
        case DKV_ERR_NOT_LEADER: return "Not leader";
        case DKV_ERR_NO_QUORUM: return "No quorum";
        case DKV_ERR_PARTITION: return "Network partition";
        case DKV_ERR_READONLY:  return "Read-only mode";
        case DKV_ERR_SHUTDOWN:  return "Shutting down";
        case DKV_ERR_IO:        return "I/O error";
        case DKV_ERR_INTERNAL:  return "Internal error";
        default:                return "Unknown error";
    }
}

#endif /* DIST_KV_TYPES_H */
