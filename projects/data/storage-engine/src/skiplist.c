#include "skiplist.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Default comparison function (lexicographic)
int default_compare(const char* a, size_t a_len,
                    const char* b, size_t b_len) {
    size_t min_len = a_len < b_len ? a_len : b_len;
    int cmp = memcmp(a, b, min_len);
    if (cmp != 0) return cmp;
    if (a_len < b_len) return -1;
    if (a_len > b_len) return 1;
    return 0;
}

// Generate random level for new node
static int random_level(void) {
    static int seeded = 0;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = 1;
    }

    int level = 1;
    while ((rand() / (double)RAND_MAX) < SKIPLIST_P &&
           level < SKIPLIST_MAX_LEVEL) {
        level++;
    }
    return level;
}

// Create a new node
static skiplist_node_t* create_node(int level, const char* key, size_t key_len,
                                    const char* value, size_t value_len) {
    skiplist_node_t* node = malloc(sizeof(skiplist_node_t));
    if (!node) return NULL;

    node->forward = malloc(sizeof(skiplist_node_t*) * level);
    if (!node->forward) {
        free(node);
        return NULL;
    }

    node->key = malloc(key_len);
    if (!node->key) {
        free(node->forward);
        free(node);
        return NULL;
    }
    memcpy(node->key, key, key_len);
    node->key_len = key_len;

    if (value && value_len > 0) {
        node->value = malloc(value_len);
        if (!node->value) {
            free(node->key);
            free(node->forward);
            free(node);
            return NULL;
        }
        memcpy(node->value, value, value_len);
        node->value_len = value_len;
    } else {
        node->value = NULL;
        node->value_len = 0;
    }

    node->deleted = false;
    node->level = level;

    for (int i = 0; i < level; i++) {
        node->forward[i] = NULL;
    }

    return node;
}

// Free a node
static void free_node(skiplist_node_t* node) {
    if (node) {
        free(node->key);
        free(node->value);
        free(node->forward);
        free(node);
    }
}

// Create a new skip list
skiplist_t* skiplist_create(compare_fn cmp) {
    skiplist_t* list = malloc(sizeof(skiplist_t));
    if (!list) return NULL;

    // Create header node with max level
    list->header = malloc(sizeof(skiplist_node_t));
    if (!list->header) {
        free(list);
        return NULL;
    }

    list->header->forward = malloc(sizeof(skiplist_node_t*) * SKIPLIST_MAX_LEVEL);
    if (!list->header->forward) {
        free(list->header);
        free(list);
        return NULL;
    }

    list->header->key = NULL;
    list->header->key_len = 0;
    list->header->value = NULL;
    list->header->value_len = 0;
    list->header->level = SKIPLIST_MAX_LEVEL;

    for (int i = 0; i < SKIPLIST_MAX_LEVEL; i++) {
        list->header->forward[i] = NULL;
    }

    list->level = 1;
    list->count = 0;
    list->memory_usage = sizeof(skiplist_t) + sizeof(skiplist_node_t) +
                         sizeof(skiplist_node_t*) * SKIPLIST_MAX_LEVEL;
    list->compare = cmp ? cmp : default_compare;

    return list;
}

// Destroy skip list and free all memory
void skiplist_destroy(skiplist_t* list) {
    if (!list) return;

    skiplist_node_t* node = list->header->forward[0];
    while (node) {
        skiplist_node_t* next = node->forward[0];
        free_node(node);
        node = next;
    }

    free(list->header->forward);
    free(list->header);
    free(list);
}

// Insert or update a key-value pair
status_t skiplist_put(skiplist_t* list, const char* key, size_t key_len,
                      const char* value, size_t value_len) {
    if (!list || !key || key_len == 0) return STATUS_INVALID_ARG;

    skiplist_node_t* update[SKIPLIST_MAX_LEVEL];
    skiplist_node_t* x = list->header;

    // Find position for insertion
    for (int i = list->level - 1; i >= 0; i--) {
        while (x->forward[i] &&
               list->compare(x->forward[i]->key, x->forward[i]->key_len,
                           key, key_len) < 0) {
            x = x->forward[i];
        }
        update[i] = x;
    }

    x = x->forward[0];

    // Key exists - update value
    if (x && list->compare(x->key, x->key_len, key, key_len) == 0) {
        // Update memory usage
        list->memory_usage -= x->value_len;

        free(x->value);
        if (value && value_len > 0) {
            x->value = malloc(value_len);
            if (!x->value) return STATUS_NO_MEMORY;
            memcpy(x->value, value, value_len);
            x->value_len = value_len;
        } else {
            x->value = NULL;
            x->value_len = 0;
        }
        x->deleted = false;

        list->memory_usage += value_len;
        return STATUS_OK;
    }

    // Insert new node
    int new_level = random_level();
    if (new_level > list->level) {
        for (int i = list->level; i < new_level; i++) {
            update[i] = list->header;
        }
        list->level = new_level;
    }

    skiplist_node_t* new_node = create_node(new_level, key, key_len, value, value_len);
    if (!new_node) return STATUS_NO_MEMORY;

    for (int i = 0; i < new_level; i++) {
        new_node->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = new_node;
    }

    list->count++;
    list->memory_usage += sizeof(skiplist_node_t) +
                          sizeof(skiplist_node_t*) * new_level +
                          key_len + value_len;

    return STATUS_OK;
}

// Get value for a key
status_t skiplist_get(skiplist_t* list, const char* key, size_t key_len,
                      char** value, size_t* value_len) {
    if (!list || !key || key_len == 0) return STATUS_INVALID_ARG;

    skiplist_node_t* x = list->header;

    for (int i = list->level - 1; i >= 0; i--) {
        while (x->forward[i] &&
               list->compare(x->forward[i]->key, x->forward[i]->key_len,
                           key, key_len) < 0) {
            x = x->forward[i];
        }
    }

    x = x->forward[0];

    if (x && list->compare(x->key, x->key_len, key, key_len) == 0) {
        if (x->deleted) {
            return STATUS_NOT_FOUND;
        }
        if (value && value_len) {
            *value = x->value;
            *value_len = x->value_len;
        }
        return STATUS_OK;
    }

    return STATUS_NOT_FOUND;
}

// Mark a key as deleted (tombstone)
status_t skiplist_delete(skiplist_t* list, const char* key, size_t key_len) {
    if (!list || !key || key_len == 0) return STATUS_INVALID_ARG;

    skiplist_node_t* x = list->header;

    for (int i = list->level - 1; i >= 0; i--) {
        while (x->forward[i] &&
               list->compare(x->forward[i]->key, x->forward[i]->key_len,
                           key, key_len) < 0) {
            x = x->forward[i];
        }
    }

    x = x->forward[0];

    if (x && list->compare(x->key, x->key_len, key, key_len) == 0) {
        x->deleted = true;
        return STATUS_OK;
    }

    // Key not found - insert tombstone
    status_t status = skiplist_put(list, key, key_len, NULL, 0);
    if (status != STATUS_OK) return status;

    // Mark the newly inserted node as deleted
    // Find the node we just inserted
    x = list->header;
    for (int i = list->level - 1; i >= 0; i--) {
        while (x->forward[i] &&
               list->compare(x->forward[i]->key, x->forward[i]->key_len,
                           key, key_len) < 0) {
            x = x->forward[i];
        }
    }
    x = x->forward[0];
    if (x && list->compare(x->key, x->key_len, key, key_len) == 0) {
        x->deleted = true;
    }
    return STATUS_OK;
}

// Check if key exists (including tombstones)
bool skiplist_contains(skiplist_t* list, const char* key, size_t key_len) {
    if (!list || !key || key_len == 0) return false;

    skiplist_node_t* x = list->header;

    for (int i = list->level - 1; i >= 0; i--) {
        while (x->forward[i] &&
               list->compare(x->forward[i]->key, x->forward[i]->key_len,
                           key, key_len) < 0) {
            x = x->forward[i];
        }
    }

    x = x->forward[0];
    return x && list->compare(x->key, x->key_len, key, key_len) == 0;
}

// Get count
size_t skiplist_count(skiplist_t* list) {
    return list ? list->count : 0;
}

// Get memory usage
size_t skiplist_memory_usage(skiplist_t* list) {
    return list ? list->memory_usage : 0;
}

// Create iterator
skiplist_iter_t* skiplist_iter_create(skiplist_t* list) {
    if (!list) return NULL;

    skiplist_iter_t* iter = malloc(sizeof(skiplist_iter_t));
    if (!iter) return NULL;

    iter->list = list;
    iter->current = NULL;

    return iter;
}

// Destroy iterator
void skiplist_iter_destroy(skiplist_iter_t* iter) {
    free(iter);
}

// Seek to first entry
void skiplist_iter_seek_to_first(skiplist_iter_t* iter) {
    if (iter && iter->list) {
        iter->current = iter->list->header->forward[0];
    }
}

// Seek to key (or first key >= target)
void skiplist_iter_seek(skiplist_iter_t* iter, const char* key, size_t key_len) {
    if (!iter || !iter->list || !key || key_len == 0) return;

    skiplist_node_t* x = iter->list->header;

    for (int i = iter->list->level - 1; i >= 0; i--) {
        while (x->forward[i] &&
               iter->list->compare(x->forward[i]->key, x->forward[i]->key_len,
                                  key, key_len) < 0) {
            x = x->forward[i];
        }
    }

    iter->current = x->forward[0];
}

// Check if iterator is valid
bool skiplist_iter_valid(skiplist_iter_t* iter) {
    return iter && iter->current != NULL;
}

// Move to next entry
void skiplist_iter_next(skiplist_iter_t* iter) {
    if (iter && iter->current) {
        iter->current = iter->current->forward[0];
    }
}

// Get current key
const char* skiplist_iter_key(skiplist_iter_t* iter, size_t* key_len) {
    if (!iter || !iter->current) return NULL;
    if (key_len) *key_len = iter->current->key_len;
    return iter->current->key;
}

// Get current value
const char* skiplist_iter_value(skiplist_iter_t* iter, size_t* value_len) {
    if (!iter || !iter->current) return NULL;
    if (value_len) *value_len = iter->current->value_len;
    return iter->current->value;
}

// Check if current entry is deleted
bool skiplist_iter_is_deleted(skiplist_iter_t* iter) {
    return iter && iter->current && iter->current->deleted;
}