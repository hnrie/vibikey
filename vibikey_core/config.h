/*
 * VibiKey - Vietnamese Input Method
 * Configuration header
 */

#ifndef VIBIKEY_CONFIG_H
#define VIBIKEY_CONFIG_H

#include <stdint.h>

/* Version info */
#define VIBIKEY_VERSION_MAJOR 1
#define VIBIKEY_VERSION_MINOR 0
#define VIBIKEY_VERSION_PATCH 0
#define VIBIKEY_VERSION_STRING "1.0.0"

/* Build configuration */
#if defined(_WIN32) && !defined(__CYGWIN__)
    #define VIBIKEY_WINDOWS 1
    #define VIBIKEY_PLATFORM_WINDOWS
#elif defined(__linux__)
    #define VIBIKEY_LINUX 1
    #define VIBIKEY_PLATFORM_LINUX
#else
    #error "Unknown platform - only Windows and Linux are supported"
#endif

/* Logging levels */
#define VIBIKEY_LOG_DEBUG 0
#define VIBIKEY_LOG_INFO  1
#define VIBIKEY_LOG_WARN  2
#define VIBIKEY_LOG_ERROR 3

extern int vibikey_log_level;

typedef void (*vibikey_log_func)(int level, const char *format, ...);

extern vibikey_log_func vibikey_log_callback;

/*
 * Set log callback
 */
void vibikey_log_set_callback(vibikey_log_func cb);

/*
 * Log function
 */
void vibikey_log(int level, const char *format, ...);

/* Backend selection */
#define VIBIKEY_BACKEND_PLATFORM 0  /* Direct platform hooks */
#define VIBIKEY_BACKEND_DELEGATE   1  /* Delegate to daemon */
#define VIBIKEY_BACKEND_PLUGIN     2  /* Plugin-based */

extern int vibikey_backend;

/* Input method modes */
#define VIBIKEY_MODE_TEIP    0
#define VIBIKEY_MODE_VNI     1
#define VIBIKEY_MODE_BYFOUR  2

extern int vibikey_im_mode;

/* Hotkey configuration */
#define VIBIKEY_HOTKEY_BACKSPACE_DEFAULT  0x63  /* 'c' by default */
#define VIBIKEY_HOTKEY_INLINE_DEFAULT     0x74  /* 't' by default */
#define VIBIKEY_HOTKEY_RESET_DEFAULT      0x72  /* 'r' by default */

#define VIBIKEY_AUTO_RESET_DELAY  1000  /* ms - auto-reset after this many consecutive keys */

/* Character buffer sizes */
#define VIBIKEY_INPUT_BUFFER_SIZE 64
#define VIBIKEY_OUTPUT_BUFFER_SIZE 256

/* Voi expressions (4-key Vietnamese accents) */
#define VIBIKEY_VOI_PREFIX 'x'

/*
 * Word buffer for inline mode
 */
typedef struct vibikey_word_buffer {
    char buffer[VIBIKEY_INPUT_BUFFER_SIZE];
    int length;
    int is_voi;
} vibikey_word_buffer_t;

/*
 * Initialize word buffer
 */
void vibikey_word_init(vibikey_word_buffer_t *buf);

/*
 * Add character to buffer
 */
void vibikey_word_add(vibikey_word_buffer_t *buf, char ch, int vomode);

/*
 * Reset word buffer
 */
void vibikey_word_reset(vibikey_word_buffer_t *buf);

/*
 * Get word from buffer
 */
const char *vibikey_word_get(vibikey_word_buffer_t *buf);

#endif /* VIBIKEY_CONFIG_H */