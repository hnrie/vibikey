/*
 * VibiKey - Vietnamese Input Method
 * Windows Platform Hook Header
 */

#ifndef VIBIKEY_WINDOWS_HOOK_H
#define VIBIKEY_WINDOWS_HOOK_H

#if defined(_WIN32) && !defined(__CYGWIN__)

#include <windows.h>
#include "../vietnamese_tep.h"
#include "../config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Hotkey definitions */
#define VIBIKEY_HOTKEY_BACKSPACE  0x63  /* 'c' */
#define VIBIKEY_HOTKEY_INLINE     0x74  /* 't' */
#define VIBIKEY_HOTKEY_RESET      0x72  /* 'r' */

/* Input mode */
typedef enum {
    VIBIKEY_INPUT_BACKSPACE = 0,
    VIBIKEY_INPUT_INLINE,
    VIBIKEY_INPUT_RAW
} vibikey_input_mode_t;

/* Global state */
extern vibikey_input_mode_t vibikey_current_mode;
extern int vibikey_hotkey_backspace;
extern int vibikey_hotkey_inline;
extern int vibikey_hotkey_reset;

/* Hook procedure IDs */
extern HHOOK g_hKeyboardHook;
extern HHOOK g_hMouseHook;

/* Input buffer */
extern char g_input_buffer[VIBIKEY_INPUT_BUFFER_SIZE];
extern int g_input_length;

/* Function prototypes */
int vibikey_windows_init(void);
int vibikey_windows_cleanup(void);
void vibikey_windows_set_mode(vibikey_input_mode_t mode);

/* Hook callbacks */
LRESULT CALLBACK vibikey_kbd_hook_proc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK vibikey_mouse_hook_proc(int nCode, WPARAM wParam, LPARAM lParam);

/* Internal input processing */
void vibikey_process_input_buffer(void);
void vibikey_reset_input_buffer(void);

#ifdef __cplusplus
}
#endif

#endif /* _WIN32 */

#endif /* VIBIKEY_WINDOWS_HOOK_H */