/*
 * VibiKey - Vietnamese Input Method
 * Input Method Selection Dialog - Common Implementation
 */

#include "vibikey_input_method_dialog.h"
#include "config.h"
#include <string.h>

/* Static method table */
static const vibikey_method_desc_t s_method_table[] = {
    {
        VIBIKEY_METHOD_BACKSPACE,
        "Backspace Method",
        "Type to buffer, press Backspace to commit",
        VIBIKEY_PLATFORM_WINDOWS_FLAG | VIBIKEY_PLATFORM_LINUX_FLAG,
        0  /* set at runtime */
    },
    {
        VIBIKEY_METHOD_INLINE,
        "Inline Method",
        "Auto-convert while typing",
        VIBIKEY_PLATFORM_WINDOWS_FLAG | VIBIKEY_PLATFORM_LINUX_FLAG,
        0
    },
    {
        VIBIKEY_METHOD_IBUS,
        "IBus",
        "Linux Input Bus framework",
        VIBIKEY_PLATFORM_LINUX_FLAG,
        0
    },
    {
        VIBIKEY_METHOD_FCITX,
        "Fcitx",
        "Flexible Input Method Framework (Linux)",
        VIBIKEY_PLATFORM_LINUX_FLAG,
        0
    }
};

static int s_method_count = sizeof(s_method_table) / sizeof(s_method_table[0]);

/*
 * Get current platform flags
 */
static uint32_t get_current_platform(void) {
#if defined(VIBIKEY_WINDOWS)
    return VIBIKEY_PLATFORM_WINDOWS_FLAG;
#elif defined(VIBIKEY_LINUX)
    return VIBIKEY_PLATFORM_LINUX_FLAG;
#else
    return 0;
#endif
}

/*
 * Get list of all input methods
 */
int vibikey_get_method_list(vibikey_method_desc_t *methods, int max_count) {
    if (!methods || max_count <= 0) {
        return 0;
    }

    uint32_t platform = get_current_platform();
    int count = 0;

    for (int i = 0; i < s_method_count && count < max_count; i++) {
        methods[count] = s_method_table[i];

        /* Check availability */
        methods[count].is_available = 
            (methods[count].supported_platforms & platform) ? 1 : 0;

        count++;
    }

    return count;
}

/*
 * Check if a method is available on current platform
 */
int vibikey_method_is_available(vibikey_method_type_t type) {
    uint32_t platform = get_current_platform();

    for (int i = 0; i < s_method_count; i++) {
        if (s_method_table[i].type == type) {
            return (s_method_table[i].supported_platforms & platform) ? 1 : 0;
        }
    }

    return 0;
}

/*
 * Initialize dialog subsystem
 */
int vibikey_dialog_init(void) {
    /* Nothing to do in common implementation */
    return 0;
}

/*
 * Cleanup dialog subsystem
 */
int vibikey_dialog_cleanup(void) {
    /* Nothing to do in common implementation */
    return 0;
}