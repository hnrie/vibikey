/*
 * VibiKey - Vietnamese Input Method
 * Linux Platform Daemon Implementation
 */

#if defined(__linux__)

#include "../config.h"
#include "../vietnamese_tep.h"
#include "vibikey_linux_daemon.h"
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <string.h>

vibikey_linux_mode_t vibikey_linux_current_mode = VIBIKEY_LINUX_BACKSPACE;
Display *vibikey_x_display = NULL;
Window vibikey_root_window = 0;

static char s_input_buffer[VIBIKEY_INPUT_BUFFER_SIZE];
static int s_input_length = 0;

/*
 * Initialize Linux daemon
 */
int vibikey_linux_init(void) {
    vibikey_x_display = XOpenDisplay(NULL);
    if (!vibikey_x_display) {
        vibikey_log(VIBIKEY_LOG_ERROR, "Failed to open X display");
        return -1;
    }

    vibikey_root_window = DefaultRootWindow(vibikey_x_display);
    vibikey_log(VIBIKEY_LOG_INFO, "Linux daemon initialized");

    return 0;
}

/*
 * Cleanup
 */
int vibikey_linux_cleanup(void) {
    if (vibikey_x_display) {
        XCloseDisplay(vibikey_x_display);
        vibikey_x_display = NULL;
    }

    vibikey_log(VIBIKEY_LOG_INFO, "Linux daemon cleaned up");
    return 0;
}

/*
 * Set input mode
 */
void vibikey_linux_set_mode(vibikey_linux_mode_t mode) {
    vibikey_linux_current_mode = mode;
    vibikey_log(VIBIKEY_LOG_INFO, "Linux input mode set to %d", mode);

    vibikey_linux_buffer_reset();
}

/*
 * Filter key event
 */
int vibikey_linux_filter_key(XKeyEvent *xevent, KeySym keysym, int is_press) {
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
        vibikey_linux_commit();
        return 0;  /* Let through */
    }

    if (ch != '\0') {
        vibikey_linux_buffer_add(ch);

        /* Inline mode: commit immediately */
        if (vibikey_linux_current_mode == VIBIKEY_LINUX_INLINE) {
            vibikey_linux_commit_inline();
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
void vibikey_linux_buffer_add(char ch) {
    if (s_input_length >= VIBIKEY_INPUT_BUFFER_SIZE - 1) {
        vibikey_linux_buffer_reset();
    }

    s_input_buffer[s_input_length++] = tolower(ch);
    s_input_buffer[s_input_length] = '\0';

    vibikey_log(VIBIKEY_LOG_DEBUG, "Linux char added: %s", s_input_buffer);
}

/*
 * Reset buffer
 */
void vibikey_linux_buffer_reset(void) {
    s_input_length = 0;
    s_input_buffer[0] = '\0';
}

/*
 * Get buffer content
 */
const char *vibikey_linux_buffer_get(void) {
    return s_input_buffer;
}

/*
 * Commit buffer (backspace method)
 */
void vibikey_linux_commit(void) {
    if (s_input_length == 0) {
        return;
    }

    char output[VIBIKEY_OUTPUT_BUFFER_SIZE];
    int method = (vibikey_im_mode == VIBIKEY_MODE_VNI) ? VIBIKEY_METHOD_VNI : VIBIKEY_METHOD_TEIP;

    int out_len = vibikey_convert_word(s_input_buffer, output, VIBIKEY_OUTPUT_BUFFER_SIZE, method);

    if (out_len > 0) {
        vibikey_log(VIBIKEY_LOG_INFO, "Commit: %s → %s", s_input_buffer, output);

        /* In full implementation, send output to focused window */
        /* For daemon, we'd use X11 SendEvent or IBus/Fcitx APIs */
    }

    vibikey_linux_buffer_reset();
}

/*
 * Commit inline (inline method)
 */
void vibikey_linux_commit_inline(void) {
    if (s_input_length == 0) {
        return;
    }

    char output[VIBIKEY_OUTPUT_BUFFER_SIZE];
    int method = (vibikey_im_mode == VIBIKEY_MODE_VNI) ? VIBIKEY_METHOD_VNI : VIBIKEY_METHOD_TEIP;

    int out_len = vibikey_convert_word(s_input_buffer, output, VIBIKEY_OUTPUT_BUFFER_SIZE, method);

    if (out_len > 0) {
        vibikey_log(VIBIKEY_LOG_DEBUG, "Inline: %s → %s", s_input_buffer, output);
    }
}

#endif /* __linux__ */