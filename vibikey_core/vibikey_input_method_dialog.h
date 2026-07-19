/*
 * VibiKey - Vietnamese Input Method
 * Input Method Selection Dialog
 * 
 * Shows available input methods for the current platform.
 * Unavailable methods are greyed out with "Unavailable for your platform"
 */

#ifndef VIBIKEY_INPUT_METHOD_DIALOG_H
#define VIBIKEY_INPUT_METHOD_DIALOG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Input method types */
typedef enum {
    VIBIKEY_METHOD_BACKSPACE = 0,   /* Backspace method (all platforms) */
    VIBIKEY_METHOD_INLINE,          /* Inline method (all platforms) */
    VIBIKEY_METHOD_IBUS,            /* IBus (Linux only) */
    VIBIKEY_METHOD_FCITX,           /* Fcitx (Linux only) */
    VIBIKEY_METHOD_MAX
} vibikey_method_type_t;

/* Platform flags */
#define VIBIKEY_PLATFORM_WINDOWS_FLAG   0x01
#define VIBIKEY_PLATFORM_LINUX_FLAG     0x02

/* Method descriptor */
typedef struct vibikey_method_desc {
    vibikey_method_type_t type;
    const char *name;
    const char *description;
    uint32_t supported_platforms;
    int is_available;       /* 1 if available on current platform */
} vibikey_method_desc_t;

/*
 * Get list of all input methods
 * 
 * @param methods: output array
 * @param max_count: size of array
 * @return: number of methods filled
 */
int vibikey_get_method_list(vibikey_method_desc_t *methods, int max_count);

/*
 * Check if a method is available on current platform
 */
int vibikey_method_is_available(vibikey_method_type_t type);

/*
 * Show the method selection dialog
 * 
 * @return: selected method type, or -1 if cancelled
 */
int vibikey_show_method_dialog(void);

/*
 * Initialize dialog subsystem
 */
int vibikey_dialog_init(void);

/*
 * Cleanup dialog subsystem
 */
int vibikey_dialog_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif /* VIBIKEY_INPUT_METHOD_DIALOG_H */