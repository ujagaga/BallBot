#include "logger.h"
#include <string.h>
#include <stdio.h>

#define LOG_ROWS 5
#define LOG_LEN  256

static char log_buf[LOG_ROWS][LOG_LEN];
static uint8_t head = 0;
static uint8_t count = 0;

void LOG_init() {
    memset(log_buf, 0, sizeof(log_buf));
    head = 0;
    count = 0;
}

void LOG_append(const char* msg) {
    if (!msg) return;

    // Copy message safely
    strncpy(log_buf[head], msg, LOG_LEN - 1);
    log_buf[head][LOG_LEN - 1] = '\0';

    head = (head + 1) % LOG_ROWS;
    if (count < LOG_ROWS) count++;
}

size_t LOG_get(char* out, size_t max_len) {
    if (!out || max_len == 0) return 0;

    size_t used = 0;
    out[0] = '\0';

    uint8_t start = (head + LOG_ROWS - count) % LOG_ROWS;

    for (uint8_t i = 0; i < count; i++) {
        uint8_t idx = (start + i) % LOG_ROWS;
        size_t len = strnlen(log_buf[idx], LOG_LEN);

        if (used + len + 1 >= max_len) break;

        memcpy(out + used, log_buf[idx], len);
        used += len;
        out[used++] = '\n';
    }

    if (used > 0) {
        out[used - 1] = '\0'; // remove last newline
        return used - 1;
    }

    out[0] = '\0';
    return 0;
}
