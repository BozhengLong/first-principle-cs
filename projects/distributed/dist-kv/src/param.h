/**
 * param.h - Tunable parameters for distributed key-value store
 *
 * Defines configurable parameters that control system behavior.
 */

#ifndef DIST_KV_PARAM_H
#define DIST_KV_PARAM_H

/* Hash ring parameters */
#define DKV_DEFAULT_VIRTUAL_NODES    150   /* Virtual nodes per physical node */
#define DKV_DEFAULT_REPLICATION      3     /* Replication factor */
#define DKV_MIN_REPLICATION          1
#define DKV_MAX_REPLICATION          7

/* Partition parameters */
#define DKV_DEFAULT_PARTITIONS       64    /* Number of partitions */
#define DKV_MIN_PARTITIONS           1
#define DKV_MAX_PARTITIONS           1024

/* Timeout parameters (milliseconds) */
#define DKV_DEFAULT_RPC_TIMEOUT      5000  /* RPC timeout */
#define DKV_DEFAULT_ELECTION_TIMEOUT 1000  /* Raft election timeout */
#define DKV_DEFAULT_HEARTBEAT_INTERVAL 100 /* Raft heartbeat interval */

/* Gossip parameters */
#define DKV_GOSSIP_INTERVAL          1000  /* Gossip interval (ms) */
#define DKV_GOSSIP_FANOUT            3     /* Number of nodes to gossip to */
#define DKV_FAILURE_THRESHOLD        3     /* Missed heartbeats before failure */
#define DKV_SUSPICION_TIMEOUT        5000  /* Time in suspicion state (ms) */

/* Storage parameters */
#define DKV_DEFAULT_MEMTABLE_SIZE    (4 * 1024 * 1024)  /* 4MB */
#define DKV_DEFAULT_BLOCK_SIZE       4096
#define DKV_DEFAULT_BLOOM_FP_RATE    0.01

/* Client parameters */
#define DKV_CLIENT_MAX_RETRIES       3
#define DKV_CLIENT_RETRY_DELAY       100   /* ms */

#endif /* DIST_KV_PARAM_H */
