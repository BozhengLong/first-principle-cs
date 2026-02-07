/**
 * rpc.h - RPC message structures
 *
 * Defines message formats for inter-node communication.
 */

#ifndef DIST_KV_RPC_H
#define DIST_KV_RPC_H

#include "types.h"
#include <stdint.h>
#include <string.h>

/* RPC message types */
typedef enum {
    RPC_PUT = 1,
    RPC_GET,
    RPC_DELETE,
    RPC_PUT_RESPONSE,
    RPC_GET_RESPONSE,
    RPC_DELETE_RESPONSE,
    RPC_FORWARD,
    RPC_FORWARD_RESPONSE,
    RPC_HEARTBEAT,
    RPC_HEARTBEAT_RESPONSE,
    RPC_VOTE_REQUEST,
    RPC_VOTE_RESPONSE,
    RPC_APPEND_ENTRIES,
    RPC_APPEND_ENTRIES_RESPONSE,
} rpc_type_t;

/* RPC message header */
typedef struct {
    uint32_t magic;          /* Magic number for validation */
    uint32_t version;        /* Protocol version */
    rpc_type_t type;         /* Message type */
    uint32_t payload_len;    /* Length of payload */
    node_id_t sender_id;     /* Sender node ID */
    uint64_t request_id;     /* Request ID for correlation */
    partition_id_t partition_id;  /* Target partition */
} rpc_header_t;

#define RPC_MAGIC 0x444B5652  /* "DKVR" */
#define RPC_VERSION 1

/* Put request */
typedef struct {
    rpc_header_t header;
    uint32_t key_len;
    uint32_t value_len;
    /* Followed by key and value data */
} rpc_put_request_t;

/* Get request */
typedef struct {
    rpc_header_t header;
    uint32_t key_len;
    consistency_level_t consistency;
    /* Followed by key data */
} rpc_get_request_t;

/* Delete request */
typedef struct {
    rpc_header_t header;
    uint32_t key_len;
    /* Followed by key data */
} rpc_delete_request_t;

/* Response status */
typedef struct {
    rpc_header_t header;
    dkv_status_t status;
    uint32_t value_len;      /* For GET responses */
    node_id_t leader_hint;   /* Hint for leader if not leader */
    /* Followed by value data for GET */
} rpc_response_t;

/* Serialize RPC header to buffer */
static inline void rpc_header_serialize(const rpc_header_t *h, uint8_t *buf) {
    memcpy(buf, &h->magic, 4);
    memcpy(buf + 4, &h->version, 4);
    memcpy(buf + 8, &h->type, 4);
    memcpy(buf + 12, &h->payload_len, 4);
    memcpy(buf + 16, &h->sender_id, 8);
    memcpy(buf + 24, &h->request_id, 8);
    memcpy(buf + 32, &h->partition_id, 4);
}

/* Deserialize RPC header from buffer */
static inline bool rpc_header_deserialize(const uint8_t *buf, rpc_header_t *h) {
    memcpy(&h->magic, buf, 4);
    if (h->magic != RPC_MAGIC) return false;
    memcpy(&h->version, buf + 4, 4);
    memcpy(&h->type, buf + 8, 4);
    memcpy(&h->payload_len, buf + 12, 4);
    memcpy(&h->sender_id, buf + 16, 8);
    memcpy(&h->request_id, buf + 24, 8);
    memcpy(&h->partition_id, buf + 32, 4);
    return true;
}

#define RPC_HEADER_SIZE 36

#endif /* DIST_KV_RPC_H */
