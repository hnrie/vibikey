/*
 * CatKey - Vietnamese Input Method
 * Configuration header
 */

#ifndef CATKEY_CONFIG_H
#define CATKEY_CONFIG_H

#include <stdint.h>

/* Version info */
#define CATKEY_VERSION_MAJOR 1
#define CATKEY_VERSION_MINOR 0
#define CATKEY_VERSION_PATCH 0
#define CATKEY_VERSION_STRING "1.0.0"

/* Build configuration */
#if defined(_WIN32) && !defined(__CYGWIN__)
    #define CATKEY_WINDOWS 1
    #define CATKEY_PLATFORM_WINDOWS
#elif defined(__linux__)
    #define CATKEY_LINUX 1
    #define CATKEY_PLATFORM_LINUX
#else
    #error "Unknown platform - only Windows and Linux are supported"
#endif

/* Logging levels */
#define CATKEY_LOG_DEBUG 0
#define CATKEY_LOG_INFO  1
#define CATKEY_LOG_WARN  2
#define CATKEY_LOG_ERROR 3

extern int catkey_log_level;

typedef void (*catkey_log_func)(int level, const char *format, ...);

extern catkey_log_func catkey_log_callback;

/*
 * Set log callback
 */
void catkey_log_set_callback(catkey_log_func cb);

/*
 * Log function
 */
void catkey_log(int level, const char *format, ...);

/* Backend selection */
#define CATKEY_BACKEND_PLATFORM 0  /* Direct platform hooks */
#define CATKEY_BACKEND_DELEGATE   1  /* Delegate to daemon */
#define CATKEY_BACKEND_PLUGIN     2  /* Plugin-based */

extern int catkey_backend;

/* Input method modes */
#define CATKEY_MODE_TEIP    0
#define CATKEY_MODE_VNI     1
#define CATKEY_MODE_BYFOUR  2

extern int catkey_im_mode;

/* Hotkey configuration */
#define CATKEY_HOTKEY_BACKSPACE_DEFAULT  0x63  /* 'c' by default */
#define CATKEY_HOTKEY_INLINE_DEFAULT     0x74  /* 't' by default */
#define CATKEY_HOTKEY_RESET_DEFAULT      0x72  /* 'r' by default */

#define CATKEY_AUTO_RESET_DELAY  1000  /* ms - auto-reset after this many consecutive keys */

/* Character buffer sizes */
#define CATKEY_INPUT_BUFFER_SIZE 64
#define CATKEY_OUTPUT_BUFFER_SIZE 256

/* Voi expressions (4-key Vietnamese accents) */
#define CATKEY_VOI_PREFIX 'x'

/*
 * Word buffer for inline mode
 */
typedef struct catkey_word_buffer {
    char buffer[CATKEY_INPUT_BUFFER_SIZE];
    int length;
    int is_voi;
} catkey_word_buffer_t;

/*
 * Initialize word buffer
 */
void catkey_word_init(catkey_word_buffer_t *buf);

/*
 * Add character to buffer
 */
void catkey_word_add(catkey_word_buffer_t *buf, char ch, int vomode);

/*
 * Reset word buffer
 */
void catkey_word_reset(catkey_word_buffer_t *buf);

/*
 * Get word from buffer
 */
const char *catkey_word_get(catkey_word_buffer_t *buf);

#endif /* CATKEY_CONFIG_H */