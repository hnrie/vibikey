/*
 * CatKey - Vietnamese Input Method
 * Linux Platform Daemon Implementation
 */

#if defined(__linux__)

#include "../config.h"
#include "../vietnamese_tep.h"
#include "catkey_linux_daemon.h"
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <string.h>

catkey_linux_mode_t catkey_linux_current_mode = CATKEY_LINUX_BACKSPACE;
Display *catkey_x_display = NULL;
Window catkey_root_window = 0;

static char s_input_buffer[CATKEY_INPUT_BUFFER_SIZE];
static int s_input_length = 0;

/*
 * Initialize Linux daemon
 */
int catkey_linux_init(void) {
    catkey_x_display = XOpenDisplay(NULL);
    if (!catkey_x_display) {
        catkey_log(CATKEY_LOG_ERROR, "Failed to open X display");
        return -1;
    }

    catkey_root_window = DefaultRootWindow(catkey_x_display);
    catkey_log(CATKEY_LOG_INFO, "Linux daemon initialized");

    return 0;
}

/*
 * Cleanup
 */
int catkey_linux_cleanup(void) {
    if (catkey_x_display) {
        XCloseDisplay(catkey_x_display);
        catkey_x_display = NULL;
    }

    catkey_log(CATKEY_LOG_INFO, "Linux daemon cleaned up");
    return 0;
}

/*
 * Set input mode
 */
void catkey_linux_set_mode(catkey_linux_mode_t mode) {
    catkey_linux_current_mode = mode;
    catkey_log(CATKEY_LOG_INFO, "Linux input mode set to %d", mode);

    catkey_linux_buffer_reset();
}

/*
 * Filter key event
 */
int catkey_linux_filter_key(XKeyEvent *xevent, KeySym keysym, int is_press) {
    if (!is_press) {
        return 0;  /* Only process key press */
    }

    /* Get the character from keysym */
    char ch = '\0';
    if (keysym >= ' ' && keysym <= '~') {
        ch = (char)keysym;
    } else if (keysym == XK_BackSpace) {
        /* Handle backspace */
        if (s_input_length > 0) {
            s_input_length--;
            s_input_buffer[s_input_length] = '\0';
        }
        return 1;  /* Consume backspace if we have buffer */
    } else if (keysym == XK_space || keysym == XK_Return) {
        /* Space/Enter commits the word */
        catkey_linux_commit();
        return 0;  /* Let through */
    }

    if (ch != '\0') {
        catkey_linux_buffer_add(ch);

        /* Inline mode: commit immediately */
        if (catkey_linux_current_mode == CATKEY_LINUX_INLINE) {
            catkey_linux_commit_inline();
            return 1;  /* Consume original char, send converted */
        }

        /* Backspace mode: keep in buffer */
        return 0;
    }

    return 0;
}

/*
 * Add character to buffer
 */
void catkey_linux_buffer_add(char ch) {
    if (s_input_length >= CATKEY_INPUT_BUFFER_SIZE - 1) {
        catkey_linux_buffer_reset();
    }

    s_input_buffer[s_input_length++] = tolower(ch);
    s_input_buffer[s_input_length] = '\0';

    catkey_log(CATKEY_LOG_DEBUG, "Linux char added: %s", s_input_buffer);
}

/*
 * Reset buffer
 */
void catkey_linux_buffer_reset(void) {
    s_input_length = 0;
    s_input_buffer[0] = '\0';
}

/*
 * Get buffer content
 */
const char *catkey_linux_buffer_get(void) {
    return s_input_buffer;
}

/*
 * Commit buffer (backspace method)
 */
void catkey_linux_commit(void) {
    if (s_input_length == 0) {
        return;
    }

    char output[CATKEY_OUTPUT_BUFFER_SIZE];
    int method = (catkey_im_mode == CATKEY_MODE_VNI) ? CATKEY_METHOD_VNI : CATKEY_METHOD_TEIP;

    int out_len = catkey_convert_word(s_input_buffer, output, CATKEY_OUTPUT_BUFFER_SIZE, method);

    if (out_len > 0) {
        catkey_log(CATKEY_LOG_INFO, "Commit: %s → %s", s_input_buffer, output);

        /* In full implementation, send output to focused window */
        /* For daemon, we'd use X11 SendEvent or IBus/Fcitx APIs */
    }

    catkey_linux_buffer_reset();
}

/*
 * Commit inline (inline method)
 */
void catkey_linux_commit_inline(void) {
    if (s_input_length == 0) {
        return;
    }

    char output[CATKEY_OUTPUT_BUFFER_SIZE];
    int method = (catkey_im_mode == CATKEY_MODE_VNI) ? CATKEY_METHOD_VNI : CATKEY_METHOD_TEIP;

    int out_len = catkey_convert_word(s_input_buffer, output, CATKEY_OUTPUT_BUFFER_SIZE, method);

    if (out_len > 0) {
        catkey_log(CATKEY_LOG_DEBUG, "Inline: %s → %s", s_input_buffer, output);
    }
}

#endif /* __linux__ */