/*
 * CatKey - Vietnamese Input Method
 * Input Method Selection Dialog - Common Implementation
 */

#include "catkey_input_method_dialog.h"
#include "config.h"
#include <string.h>

/* Static method table */
static const catkey_method_desc_t s_method_table[] = {
    {
        CATKEY_METHOD_BACKSPACE,
        "Backspace Method",
        "Type to buffer, press Backspace to commit",
        CATKEY_PLATFORM_WINDOWS_FLAG | CATKEY_PLATFORM_LINUX_FLAG,
        0  /* set at runtime */
    },
    {
        CATKEY_METHOD_INLINE,
        "Inline Method",
        "Auto-convert while typing",
        CATKEY_PLATFORM_WINDOWS_FLAG | CATKEY_PLATFORM_LINUX_FLAG,
        0
    },
    {
        CATKEY_METHOD_IBUS,
        "IBus",
        "Linux Input Bus framework",
        CATKEY_PLATFORM_LINUX_FLAG,
        0
    },
    {
        CATKEY_METHOD_FCITX,
        "Fcitx",
        "Flexible Input Method Framework (Linux)",
        CATKEY_PLATFORM_LINUX_FLAG,
        0
    }
};

static int s_method_count = sizeof(s_method_table) / sizeof(s_method_table[0]);

/*
 * Get current platform flags
 */
static uint32_t get_current_platform(void) {
#if defined(CATKEY_WINDOWS)
    return CATKEY_PLATFORM_WINDOWS_FLAG;
#elif defined(CATKEY_LINUX)
    return CATKEY_PLATFORM_LINUX_FLAG;
#else
    return 0;
#endif
}

/*
 * Get list of all input methods
 */
int catkey_get_method_list(catkey_method_desc_t *methods, int max_count) {
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
int catkey_method_is_available(catkey_method_type_t type) {
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
int catkey_dialog_init(void) {
    /* Nothing to do in common implementation */
    return 0;
}

/*
 * Cleanup dialog subsystem
 */
int catkey_dialog_cleanup(void) {
    /* Nothing to do in common implementation */
    return 0;
}