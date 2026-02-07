#ifndef CRC32_H
#define CRC32_H

#include <stdint.h>
#include <stddef.h>

// Calculate CRC32 checksum for data
uint32_t crc32(const void* data, size_t len);

// Update CRC32 incrementally (for streaming data)
uint32_t crc32_update(uint32_t crc, const void* data, size_t len);

#endif // CRC32_H
