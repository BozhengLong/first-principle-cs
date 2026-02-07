#include "manifest.h"
#include "crc32.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

#define MANIFEST_FILENAME "MANIFEST"

// Helper: write all bytes
static ssize_t write_all(int fd, const void* buf, size_t len) {
    const uint8_t* p = buf;
    size_t remaining = len;
    while (remaining > 0) {
        ssize_t n = write(fd, p, remaining);
        if (n <= 0) return -1;
        p += n;
        remaining -= n;
    }
    return (ssize_t)len;
}

// Helper: read all bytes
static ssize_t read_all(int fd, void* buf, size_t len) {
    uint8_t* p = buf;
    size_t remaining = len;
    while (remaining > 0) {
        ssize_t n = read(fd, p, remaining);
        if (n <= 0) return n == 0 ? (ssize_t)(len - remaining) : -1;
        p += n;
        remaining -= n;
    }
    return (ssize_t)len;
}

// Helper: build manifest path
static char* manifest_path(const char* db_path) {
    size_t len = strlen(db_path) + strlen(MANIFEST_FILENAME) + 2;
    char* path = malloc(len);
    if (path) {
        snprintf(path, len, "%s/%s", db_path, MANIFEST_FILENAME);
    }
    return path;
}

// Create manifest file
status_t manifest_create(const char* db_path) {
    if (!db_path) return STATUS_INVALID_ARG;

    char* path = manifest_path(db_path);
    if (!path) return STATUS_NO_MEMORY;

    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    free(path);

    if (fd < 0) return STATUS_IO_ERROR;
    close(fd);

    return STATUS_OK;
}

// Helper: append a record to manifest
static status_t manifest_append(const char* db_path, uint8_t type,
                                const void* data, size_t data_len) {
    char* path = manifest_path(db_path);
    if (!path) return STATUS_NO_MEMORY;

    int fd = open(path, O_WRONLY | O_APPEND | O_CREAT, 0644);
    free(path);

    if (fd < 0) return STATUS_IO_ERROR;

    // Record format: CRC32(4) + Type(1) + Len(4) + Data
    size_t record_size = 9 + data_len;
    uint8_t* record = malloc(record_size);
    if (!record) {
        close(fd);
        return STATUS_NO_MEMORY;
    }

    // Write type and length
    record[4] = type;
    uint32_t len32 = (uint32_t)data_len;
    memcpy(record + 5, &len32, 4);

    // Write data
    if (data_len > 0) {
        memcpy(record + 9, data, data_len);
    }

    // Compute CRC over type + len + data
    uint32_t crc = crc32(record + 4, 5 + data_len);
    memcpy(record, &crc, 4);

    ssize_t written = write_all(fd, record, record_size);
    free(record);
    close(fd);

    return (written == (ssize_t)record_size) ? STATUS_OK : STATUS_IO_ERROR;
}

// Log add file
status_t manifest_log_add_file(const char* db_path, int level, uint64_t file_num) {
    uint8_t data[12];
    uint32_t level32 = (uint32_t)level;
    memcpy(data, &level32, 4);
    memcpy(data + 4, &file_num, 8);
    return manifest_append(db_path, MANIFEST_ADD_FILE, data, 12);
}

// Log remove file
status_t manifest_log_remove_file(const char* db_path, int level, uint64_t file_num) {
    uint8_t data[12];
    uint32_t level32 = (uint32_t)level;
    memcpy(data, &level32, 4);
    memcpy(data + 4, &file_num, 8);
    return manifest_append(db_path, MANIFEST_REMOVE_FILE, data, 12);
}

// Log next file number
status_t manifest_log_next_file_num(const char* db_path, uint64_t next_num) {
    return manifest_append(db_path, MANIFEST_NEXT_FILE_NUM, &next_num, 8);
}

// Recover from manifest
status_t manifest_recover(const char* db_path, level_manager_t* lm) {
    if (!db_path || !lm) return STATUS_INVALID_ARG;

    char* path = manifest_path(db_path);
    if (!path) return STATUS_NO_MEMORY;

    int fd = open(path, O_RDONLY);
    free(path);

    if (fd < 0) {
        // No manifest file - scan directory for SST files
        DIR* dir = opendir(db_path);
        if (!dir) return STATUS_OK;

        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            size_t name_len = strlen(entry->d_name);
            if (name_len > 4 && strcmp(entry->d_name + name_len - 4, ".sst") == 0) {
                uint64_t file_num = strtoull(entry->d_name, NULL, 10);

                char sst_path[512];
                snprintf(sst_path, sizeof(sst_path), "%s/%s", db_path, entry->d_name);

                sstable_reader_t* reader = sstable_reader_open(sst_path, lm->cmp);
                if (reader) {
                    // Add to L0 by default when no manifest
                    level_add_sstable(lm, 0, file_num, sst_path, reader);
                }
            }
        }
        closedir(dir);
        return STATUS_OK;
    }

    // Read manifest records
    while (1) {
        uint8_t header[9];
        ssize_t n = read_all(fd, header, 9);
        if (n < 9) break;  // EOF or error

        uint32_t stored_crc;
        memcpy(&stored_crc, header, 4);

        uint8_t type = header[4];
        uint32_t data_len;
        memcpy(&data_len, header + 5, 4);

        uint8_t* data = NULL;
        if (data_len > 0) {
            data = malloc(data_len);
            if (!data) {
                close(fd);
                return STATUS_NO_MEMORY;
            }
            if (read_all(fd, data, data_len) != (ssize_t)data_len) {
                free(data);
                break;
            }
        }

        // Verify CRC
        uint8_t* check_buf = malloc(5 + data_len);
        if (!check_buf) {
            free(data);
            close(fd);
            return STATUS_NO_MEMORY;
        }
        memcpy(check_buf, header + 4, 5);
        if (data_len > 0) {
            memcpy(check_buf + 5, data, data_len);
        }
        uint32_t computed_crc = crc32(check_buf, 5 + data_len);
        free(check_buf);

        if (computed_crc != stored_crc) {
            free(data);
            close(fd);
            return STATUS_CORRUPTION;
        }

        // Process record
        switch (type) {
            case MANIFEST_ADD_FILE: {
                if (data_len >= 12) {
                    uint32_t level;
                    uint64_t file_num;
                    memcpy(&level, data, 4);
                    memcpy(&file_num, data + 4, 8);

                    char sst_path[512];
                    snprintf(sst_path, sizeof(sst_path), "%s/%06llu.sst",
                             db_path, (unsigned long long)file_num);

                    sstable_reader_t* reader = sstable_reader_open(sst_path, lm->cmp);
                    if (reader) {
                        level_add_sstable(lm, (int)level, file_num, sst_path, reader);
                    }
                }
                break;
            }
            case MANIFEST_REMOVE_FILE: {
                if (data_len >= 12) {
                    uint32_t level;
                    uint64_t file_num;
                    memcpy(&level, data, 4);
                    memcpy(&file_num, data + 4, 8);
                    level_remove_sstable(lm, (int)level, file_num);
                }
                break;
            }
            case MANIFEST_NEXT_FILE_NUM: {
                if (data_len >= 8) {
                    uint64_t next_num;
                    memcpy(&next_num, data, 8);
                    level_set_next_file_number(lm, next_num);
                }
                break;
            }
        }

        free(data);
    }

    close(fd);
    return STATUS_OK;
}
