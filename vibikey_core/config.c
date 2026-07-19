/*
 * VibiKey - Vietnamese Input Method
 * Configuration implementation
 */

#include "config.h"
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>

int vibikey_log_level = VIBIKEY_LOG_INFO;
vibikey_log_func vibikey_log_callback = NULL;

void vibikey_log_set_callback(vibikey_log_func cb) {
    vibikey_log_callback = cb;
}

void vibikey_log(int level, const char *format, ...) {
    if (level < vibikey_log_level) {
        return;
    }

    const char *level_str[] = { "DEBUG", "INFO", "WARN", "ERROR" };
    const char *prefix = (level >= 0 && level <= 3) ? level_str[level] : "UNKNOWN";

    if (vibikey_log_callback) {
        va_list args;
        va_start(args, format);
        vibikey_log_callback(level, format, args);
        va_end(args);
        return;
    }

    fprintf(stderr, "[VIBIKEY][%s] ", prefix);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    fprintf(stderr, "\n");
}

void vibikey_word_init(vibikey_word_buffer_t *buf) {
    if (buf) {
        buf->length = 0;
        buf->is_voi = 0;
        buf->buffer[0] = '\0';
    }
}

void vibikey_word_add(vibikey_word_buffer_t *buf, char ch, int vomode) {
    if (!buf || buf->length >= VIBIKEY_INPUT_BUFFER_SIZE - 1) {
        return;
    }

    buf->buffer[buf->length++] = tolower(ch);

    if (vomode) {
        buf->is_voi = 1;
    }

    buf->buffer[buf->length] = '\0';
}

void vibikey_word_reset(vibikey_word_buffer_t *buf) {
    if (buf) {
        buf->length = 0;
        buf->is_voi = 0;
        buf->buffer[0] = '\0';
    }
}

const char *vibikey_word_get(vibikey_word_buffer_t *buf) {
    return (buf) ? buf->buffer : "";
}