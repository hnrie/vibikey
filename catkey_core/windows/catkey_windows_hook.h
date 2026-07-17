/*
 * CatKey - Vietnamese Input Method
 * Windows Platform Hook Header
 */

#ifndef CATKEY_WINDOWS_HOOK_H
#define CATKEY_WINDOWS_HOOK_H

#if defined(_WIN32) && !defined(__CYGWIN__)

#include <windows.h>
#include "../vietnamese_tep.h"
#include "../config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Hotkey definitions */
#define CATKEY_HOTKEY_BACKSPACE  0x63  /* 'c' */
#define CATKEY_HOTKEY_INLINE     0x74  /* 't' */
#define CATKEY_HOTKEY_RESET      0x72  /* 'r' */

/* Input mode */
typedef enum {
    CATKEY_INPUT_BACKSPACE = 0,
    CATKEY_INPUT_INLINE,
    CATKEY_INPUT_RAW
} catkey_input_mode_t;

/* Global state */
extern catkey_input_mode_t catkey_current_mode;
extern int catkey_hotkey_backspace;
extern int catkey_hotkey_inline;
extern int catkey_hotkey_reset;

/* Hook procedure IDs */
extern HHOOK g_hKeyboardHook;
extern HHOOK g_hMouseHook;

/* Input buffer */
extern char g_input_buffer[CATKEY_INPUT_BUFFER_SIZE];
extern int g_input_length;

/* Function prototypes */
int catkey_windows_init(void);
int catkey_windows_cleanup(void);
void catkey_windows_set_mode(catkey_input_mode_t mode);

/* Hook callbacks */
LRESULT CALLBACK catkey_kbd_hook_proc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK catkey_mouse_hook_proc(int nCode, WPARAM wParam, LPARAM lParam);

/* Internal input processing */
void catkey_process_input_buffer(void);
void catkey_reset_input_buffer(void);

#ifdef __cplusplus
}
#endif

#endif /* _WIN32 */

#endif /* CATKEY_WINDOWS_HOOK_H */