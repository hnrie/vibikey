/*
 * CatKey - Vietnamese Input Method
 * Linux Platform Daemon
 * 
 * Common daemon logic that handles:
 *   - Backspace method (type buffer, press Backspace to commit)
 *   - Inline method (auto-convert while typing)
 *   - Will be wrapped by IBus/Fcitx modules
 */

#ifndef CATKEY_LINUX_DAEMON_H
#define CATKEY_LINUX_DAEMON_H

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
    CATKEY_LINUX_BACKSPACE = 0,
    CATKEY_LINUX_INLINE,
    CATKEY_LINUX_RAW
} catkey_linux_mode_t;

extern catkey_linux_mode_t catkey_linux_current_mode;

/* X11 display connection */
extern Display *catkey_x_display;
extern Window catkey_root_window;

/* Function prototypes */
int catkey_linux_init(void);
int catkey_linux_cleanup(void);
void catkey_linux_set_mode(catkey_linux_mode_t mode);

/* Key event filtering */
int catkey_linux_filter_key(XKeyEvent *xevent, KeySym keysym, int is_press);

/* Buffer management */
void catkey_linux_buffer_add(char ch);
void catkey_linux_buffer_reset(void);
const char *catkey_linux_buffer_get(void);

/* Process and commit buffer */
void catkey_linux_commit(void);
void catkey_linux_commit_inline(void);

#ifdef __cplusplus
}
#endif

#endif /* __linux__ */

#endif /* CATKEY_LINUX_DAEMON_H */