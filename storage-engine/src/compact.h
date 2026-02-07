#ifndef STORAGE_COMPACT_H
#define STORAGE_COMPACT_H

#include "types.h"
#include "level.h"
#include "sstable.h"

// Forward declaration for SSTable iterator
typedef struct sstable_iter sstable_iter_t;

// SSTable iterator API (extends sstable.h)
sstable_iter_t* sstable_iter_create(sstable_reader_t* reader);
void sstable_iter_destroy(sstable_iter_t* iter);
void sstable_iter_seek_to_first(sstable_iter_t* iter);
bool sstable_iter_valid(sstable_iter_t* iter);
void sstable_iter_next(sstable_iter_t* iter);
const char* sstable_iter_key(sstable_iter_t* iter, size_t* len);
const char* sstable_iter_value(sstable_iter_t* iter, size_t* len);
bool sstable_iter_is_deleted(sstable_iter_t* iter);

// Compaction API
status_t compact_level(level_manager_t* lm, int level);

// Check if any level needs compaction
int compact_pick_level(level_manager_t* lm);

#endif // STORAGE_COMPACT_H
