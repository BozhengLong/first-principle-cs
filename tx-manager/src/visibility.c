/**
 * visibility.c - MVCC Version Visibility Implementation
 */

#include "visibility.h"
#include "tx_manager.h"
#include <stdint.h>

bool is_version_visible(uint64_t version_ts, tx_t* tx) {
    if (!tx) return false;

    /* Version must be committed before our snapshot */
    return version_ts <= tx->start_ts;
}

uint64_t get_min_active_snapshot(tx_manager_t* tm) {
    if (!tm) return UINT64_MAX;

    pthread_mutex_lock(&tm->mutex);

    uint64_t min_ts = UINT64_MAX;
    for (size_t i = 0; i < tm->active_capacity; i++) {
        if (tm->active_txs[i] && tm->active_txs[i]->start_ts < min_ts) {
            min_ts = tm->active_txs[i]->start_ts;
        }
    }

    pthread_mutex_unlock(&tm->mutex);
    return min_ts;
}
