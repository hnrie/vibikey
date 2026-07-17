/*
 * CatKey - Vietnamese Input Method
 * Input Method Selection Dialog
 * 
 * Shows available input methods for the current platform.
 * Unavailable methods are greyed out with "Unavailable for your platform"
 */

#ifndef CATKEY_INPUT_METHOD_DIALOG_H
#define CATKEY_INPUT_METHOD_DIALOG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Input method types */
typedef enum {
    CATKEY_METHOD_BACKSPACE = 0,   /* Backspace method (all platforms) */
    CATKEY_METHOD_INLINE,          /* Inline method (all platforms) */
    CATKEY_METHOD_IBUS,            /* IBus (Linux only) */
    CATKEY_METHOD_FCITX,           /* Fcitx (Linux only) */
    CATKEY_METHOD_MAX
} catkey_method_type_t;

/* Platform flags */
#define CATKEY_PLATFORM_WINDOWS_FLAG   0x01
#define CATKEY_PLATFORM_LINUX_FLAG     0x02

/* Method descriptor */
typedef struct catkey_method_desc {
    catkey_method_type_t type;
    const char *name;
    const char *description;
    uint32_t supported_platforms;
    int is_available;       /* 1 if available on current platform */
} catkey_method_desc_t;

/*
 * Get list of all input methods
 * 
 * @param methods: output array
 * @param max_count: size of array
 * @return: number of methods filled
 */
int catkey_get_method_list(catkey_method_desc_t *methods, int max_count);

/*
 * Check if a method is available on current platform
 */
int catkey_method_is_available(catkey_method_type_t type);

/*
 * Show the method selection dialog
 * 
 * @return: selected method type, or -1 if cancelled
 */
int catkey_show_method_dialog(void);

/*
 * Initialize dialog subsystem
 */
int catkey_dialog_init(void);

/*
 * Cleanup dialog subsystem
 */
int catkey_dialog_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif /* CATKEY_INPUT_METHOD_DIALOG_H */