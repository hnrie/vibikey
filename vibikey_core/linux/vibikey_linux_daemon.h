/*
 * VibiKey - Vietnamese Input Method
 * Linux Platform Daemon
 * 
 * Common daemon logic that handles:
 *   - Backspace method (type buffer, press Backspace to commit)
 *   - Inline method (auto-convert while typing)
 *   - Will be wrapped by IBus/Fcitx modules
 */

#ifndef VIBIKEY_LINUX_DAEMON_H
#define VIBIKEY_LINUX_DAEMON_H

#if defined(__linux__)

#include <stdint.h>
#include <X11/Xlib.h>
#include "../vietnamese_tep.h"
#include "../config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Linux daemon states */
typedef enum {
    VIBIKEY_LINUX_BACKSPACE = 0,
    VIBIKEY_LINUX_INLINE,
    VIBIKEY_LINUX_RAW
} vibikey_linux_mode_t;

extern vibikey_linux_mode_t vibikey_linux_current_mode;

/* X11 display connection */
extern Display *vibikey_x_display;
extern Window vibikey_root_window;

/* Function prototypes */
int vibikey_linux_init(void);
int vibikey_linux_cleanup(void);
void vibikey_linux_set_mode(vibikey_linux_mode_t mode);

/* Key event filtering */
int vibikey_linux_filter_key(XKeyEvent *xevent, KeySym keysym, int is_press);

/* Buffer management */
void vibikey_linux_buffer_add(char ch);
void vibikey_linux_buffer_reset(void);
const char *vibikey_linux_buffer_get(void);

/* Process and commit buffer */
void vibikey_linux_commit(void);
void vibikey_linux_commit_inline(void);

#ifdef __cplusplus
}
#endif

#endif /* __linux__ */

#endif /* VIBIKEY_LINUX_DAEMON_H */