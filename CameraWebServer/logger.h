#pragma once
#include <stddef.h>

void LOG_init();
void LOG_append(const char* msg);

/**
 * Copies logs into provided buffer as a single newline-separated string.
 * Buffer is null-terminated.
 * Returns number of bytes written (excluding null).
 */
size_t LOG_get(char* out, size_t max_len);
