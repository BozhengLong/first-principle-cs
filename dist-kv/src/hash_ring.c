/**
 * hash_ring.c - Consistent hashing implementation
 */

#include "hash_ring.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* MurmurHash3 finalizer for 64-bit */
static uint64_t murmur_mix(uint64_t k) {
    k ^= k >> 33;
    k *= 0xff51afd7ed558ccdULL;
    k ^= k >> 33;
    k *= 0xc4ceb9fe1a85ec53ULL;
    k ^= k >> 33;
    return k;
}

/* Hash function using MurmurHash3-like mixing */
uint64_t hash_ring_hash_key(const char *key, size_t key_len) {
    uint64_t h = 0x9e3779b97f4a7c15ULL;  /* Golden ratio */

    const uint64_t *blocks = (const uint64_t *)key;
    size_t nblocks = key_len / 8;

    for (size_t i = 0; i < nblocks; i++) {
        uint64_t k;
        memcpy(&k, &blocks[i], sizeof(k));
        h ^= murmur_mix(k);
        h = (h << 27) | (h >> 37);
        h = h * 5 + 0x52dce729;
    }

    /* Handle remaining bytes */
    const uint8_t *tail = (const uint8_t *)(key + nblocks * 8);
    uint64_t k = 0;
    switch (key_len & 7) {
        case 7: k ^= (uint64_t)tail[6] << 48; /* fallthrough */
        case 6: k ^= (uint64_t)tail[5] << 40; /* fallthrough */
        case 5: k ^= (uint64_t)tail[4] << 32; /* fallthrough */
        case 4: k ^= (uint64_t)tail[3] << 24; /* fallthrough */
        case 3: k ^= (uint64_t)tail[2] << 16; /* fallthrough */
        case 2: k ^= (uint64_t)tail[1] << 8;  /* fallthrough */
        case 1: k ^= (uint64_t)tail[0];
                h ^= murmur_mix(k);
    }

    return murmur_mix(h);
}

/* Hash ring structure */
struct hash_ring {
    hash_ring_config_t config;
    vnode_t *vnodes;         /* Sorted array of virtual nodes */
    int vnode_count;         /* Current number of virtual nodes */
    int vnode_capacity;      /* Allocated capacity */
    node_id_t *nodes;        /* Array of physical node IDs */
    int node_count;          /* Number of physical nodes */
    int node_capacity;       /* Allocated capacity for nodes */
};

/* Compare function for sorting vnodes by hash */
static int vnode_compare(const void *a, const void *b) {
    const vnode_t *va = (const vnode_t *)a;
    const vnode_t *vb = (const vnode_t *)b;
    if (va->hash < vb->hash) return -1;
    if (va->hash > vb->hash) return 1;
    return 0;
}

/* Generate hash for a virtual node */
static uint64_t vnode_hash(node_id_t node_id, int vnode_index) {
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "%lu:%d", (unsigned long)node_id, vnode_index);
    return hash_ring_hash_key(buf, len);
}

hash_ring_t *hash_ring_create(const hash_ring_config_t *config) {
    hash_ring_t *ring = calloc(1, sizeof(hash_ring_t));
    if (!ring) return NULL;

    if (config) {
        ring->config = *config;
    } else {
        ring->config.num_virtual_nodes = DKV_DEFAULT_VIRTUAL_NODES;
        ring->config.replication_factor = DKV_DEFAULT_REPLICATION;
    }

    /* Validate config */
    if (ring->config.num_virtual_nodes < 1) {
        ring->config.num_virtual_nodes = DKV_DEFAULT_VIRTUAL_NODES;
    }
    if (ring->config.replication_factor < DKV_MIN_REPLICATION) {
        ring->config.replication_factor = DKV_MIN_REPLICATION;
    }
    if (ring->config.replication_factor > DKV_MAX_REPLICATION) {
        ring->config.replication_factor = DKV_MAX_REPLICATION;
    }

    ring->vnode_capacity = 16;
    ring->vnodes = malloc(ring->vnode_capacity * sizeof(vnode_t));
    if (!ring->vnodes) {
        free(ring);
        return NULL;
    }

    ring->node_capacity = 8;
    ring->nodes = malloc(ring->node_capacity * sizeof(node_id_t));
    if (!ring->nodes) {
        free(ring->vnodes);
        free(ring);
        return NULL;
    }

    return ring;
}

void hash_ring_destroy(hash_ring_t *ring) {
    if (!ring) return;
    free(ring->vnodes);
    free(ring->nodes);
    free(ring);
}

bool hash_ring_has_node(hash_ring_t *ring, node_id_t node_id) {
    if (!ring) return false;
    for (int i = 0; i < ring->node_count; i++) {
        if (ring->nodes[i] == node_id) return true;
    }
    return false;
}

int hash_ring_node_count(hash_ring_t *ring) {
    return ring ? ring->node_count : 0;
}

dkv_status_t hash_ring_add_node(hash_ring_t *ring, node_id_t node_id) {
    if (!ring) return DKV_ERR_INVALID;
    if (hash_ring_has_node(ring, node_id)) return DKV_ERR_EXISTS;

    /* Expand node array if needed */
    if (ring->node_count >= ring->node_capacity) {
        int new_cap = ring->node_capacity * 2;
        node_id_t *new_nodes = realloc(ring->nodes, new_cap * sizeof(node_id_t));
        if (!new_nodes) return DKV_ERR_NOMEM;
        ring->nodes = new_nodes;
        ring->node_capacity = new_cap;
    }

    /* Expand vnode array if needed */
    int new_vnodes = ring->config.num_virtual_nodes;
    if (ring->vnode_count + new_vnodes > ring->vnode_capacity) {
        int new_cap = ring->vnode_capacity;
        while (new_cap < ring->vnode_count + new_vnodes) {
            new_cap *= 2;
        }
        vnode_t *new_arr = realloc(ring->vnodes, new_cap * sizeof(vnode_t));
        if (!new_arr) return DKV_ERR_NOMEM;
        ring->vnodes = new_arr;
        ring->vnode_capacity = new_cap;
    }

    /* Add physical node */
    ring->nodes[ring->node_count++] = node_id;

    /* Add virtual nodes */
    for (int i = 0; i < ring->config.num_virtual_nodes; i++) {
        vnode_t *vn = &ring->vnodes[ring->vnode_count++];
        vn->hash = vnode_hash(node_id, i);
        vn->node_id = node_id;
        vn->vnode_index = i;
    }

    /* Sort vnodes by hash */
    qsort(ring->vnodes, ring->vnode_count, sizeof(vnode_t), vnode_compare);

    return DKV_OK;
}

dkv_status_t hash_ring_remove_node(hash_ring_t *ring, node_id_t node_id) {
    if (!ring) return DKV_ERR_INVALID;

    /* Find and remove physical node */
    int found = -1;
    for (int i = 0; i < ring->node_count; i++) {
        if (ring->nodes[i] == node_id) {
            found = i;
            break;
        }
    }
    if (found < 0) return DKV_ERR_NOT_FOUND;

    /* Remove from nodes array */
    memmove(&ring->nodes[found], &ring->nodes[found + 1],
            (ring->node_count - found - 1) * sizeof(node_id_t));
    ring->node_count--;

    /* Remove virtual nodes */
    int write_idx = 0;
    for (int i = 0; i < ring->vnode_count; i++) {
        if (ring->vnodes[i].node_id != node_id) {
            if (write_idx != i) {
                ring->vnodes[write_idx] = ring->vnodes[i];
            }
            write_idx++;
        }
    }
    ring->vnode_count = write_idx;

    return DKV_OK;
}

/* Binary search for the first vnode with hash >= target */
static int find_vnode(hash_ring_t *ring, uint64_t hash) {
    if (ring->vnode_count == 0) return -1;

    int lo = 0, hi = ring->vnode_count;
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (ring->vnodes[mid].hash < hash) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }

    /* Wrap around to first vnode if we're past the end */
    if (lo >= ring->vnode_count) {
        lo = 0;
    }
    return lo;
}

node_id_t hash_ring_get_node(hash_ring_t *ring, const char *key, size_t key_len) {
    if (!ring || ring->vnode_count == 0) return 0;

    uint64_t hash = hash_ring_hash_key(key, key_len);
    int idx = find_vnode(ring, hash);
    if (idx < 0) return 0;

    return ring->vnodes[idx].node_id;
}

int hash_ring_get_replicas(hash_ring_t *ring, const char *key, size_t key_len,
                           node_id_t *nodes, int max_nodes) {
    if (!ring || !nodes || max_nodes <= 0 || ring->vnode_count == 0) {
        return 0;
    }

    uint64_t hash = hash_ring_hash_key(key, key_len);
    int start_idx = find_vnode(ring, hash);
    if (start_idx < 0) return 0;

    int count = 0;
    int target = ring->config.replication_factor;
    if (target > max_nodes) target = max_nodes;
    if (target > ring->node_count) target = ring->node_count;

    /* Walk the ring, collecting unique nodes */
    for (int i = 0; i < ring->vnode_count && count < target; i++) {
        int idx = (start_idx + i) % ring->vnode_count;
        node_id_t node = ring->vnodes[idx].node_id;

        /* Check if we already have this node */
        bool duplicate = false;
        for (int j = 0; j < count; j++) {
            if (nodes[j] == node) {
                duplicate = true;
                break;
            }
        }

        if (!duplicate) {
            nodes[count++] = node;
        }
    }

    return count;
}

void hash_ring_get_stats(hash_ring_t *ring, hash_ring_stats_t *stats) {
    if (!ring || !stats) return;

    stats->total_vnodes = ring->vnode_count;
    stats->node_count = ring->node_count;

    if (ring->node_count == 0) {
        stats->avg_vnodes = 0;
        stats->std_dev = 0;
        return;
    }

    stats->avg_vnodes = (double)ring->vnode_count / ring->node_count;

    /* Calculate standard deviation */
    double sum_sq = 0;
    for (int i = 0; i < ring->node_count; i++) {
        int count = 0;
        for (int j = 0; j < ring->vnode_count; j++) {
            if (ring->vnodes[j].node_id == ring->nodes[i]) {
                count++;
            }
        }
        double diff = count - stats->avg_vnodes;
        sum_sq += diff * diff;
    }
    stats->std_dev = (ring->node_count > 1) ?
        sqrt(sum_sq / ring->node_count) : 0;
}
