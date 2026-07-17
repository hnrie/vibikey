/*
 * CatKey - Vietnamese Input Method
 * Windows Platform Hook Implementation
 * 
 * Hotkey modes:
 *   - Backspace method: Type to buffer, press Backspace to commit
 *   - Inline method: Type normally, auto-convert inline
 *   - Reset: Clear current buffer
 */

#if defined(_WIN32) && !defined(__CYGWIN__)

#include "../config.h"
#include "../vietnamese_tep.h"
#include "catkey_windows_hook.h"
#include <stdio.h>

/* Global state */
catkey_input_mode_t catkey_current_mode = CATKEY_INPUT_BACKSPACE;
int catkey_hotkey_backspace = CATKEY_HOTKEY_BACKSPACE;
int catkey_hotkey_inline = CATKEY_HOTKEY_INLINE;
int catkey_hotkey_reset = CATKEY_HOTKEY_RESET;

HHOOK g_hKeyboardHook = NULL;
HHOOK g_hMouseHook = NULL;

char g_input_buffer[CATKEY_INPUT_BUFFER_SIZE];
int g_input_length = 0;

/* Internal hooks */
static HHOOK s_hLowLevelKeyboard = NULL;

/* Forward declarations */
static void process_hotkey(DWORD vkCode);
static int is_terminator_key(DWORD vkCode);
static int is_vietnamese_char(char ch);
static void process_character_input(char ch);
static void process_inline_buffer(void);

/*
 * Initialize Windows hooks
 */
int catkey_windows_init(void) {
    g_hKeyboardHook = SetWindowsHookEx(
        WH_KEYBOARD_LL,           /* Low-level keyboard hook */
        catkey_kbd_hook_proc,
        GetModuleHandle(NULL),
        0
    );

    if (g_hKeyboardHook == NULL) {
        catkey_log(CATKEY_LOG_ERROR, "Failed to install keyboard hook (error %lu)", GetLastError());
        return -1;
    }

    catkey_log(CATKEY_LOG_INFO, "Windows keyboard hook installed successfully");
    return 0;
}

/*
 * Cleanup hooks
 */
int catkey_windows_cleanup(void) {
    if (g_hKeyboardHook) {
        UnhookWindowsHookEx(g_hKeyboardHook);
        g_hKeyboardHook = NULL;
    }
    if (g_hMouseHook) {
        UnhookWindowsHookEx(g_hMouseHook);
        g_hMouseHook = NULL;
    }

    catkey_log(CATKEY_LOG_INFO, "Windows hooks removed");
    return 0;
}

/*
 * Set input mode
 */
void catkey_windows_set_mode(catkey_input_mode_t mode) {
    catkey_current_mode = mode;
    catkey_log(CATKEY_LOG_INFO, "Input mode set to %d", mode);

    /* Reset buffer when switching modes */
    catkey_reset_input_buffer();
}

/*
 * Low-level keyboard hook callback
 */
LRESULT CALLBACK catkey_kbd_hook_proc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        KBDLLHOOKSTRUCT *pkb = (KBDLLHOOKSTRUCT *)lParam;

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            DWORD vkCode = pkb->vkCode;

            /* Check for hotkeys */
            if (vkCode == catkey_hotkey_backspace) {
                catkey_log(CATKEY_LOG_DEBUG, "Backspace hotkey pressed");
                catkey_windows_set_mode(CATKEY_INPUT_BACKSPACE);
                return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
            }

            if (vkCode == catkey_hotkey_inline) {
                catkey_log(CATKEY_LOG_DEBUG, "Inline hotkey pressed");
                catkey_windows_set_mode(CATKEY_INPUT_INLINE);
                return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
            }

            if (vkCode == catkey_hotkey_reset) {
                catkey_log(CATKEY_LOG_DEBUG, "Reset hotkey pressed");
                catkey_reset_input_buffer();
                return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
            }

            /* Handle normal input */
            if (vkCode == VK_BACK) {
                /* Remove last char from buffer */
                if (g_input_length > 0) {
                    g_input_length--;
                    g_input_buffer[g_input_length] = '\0';
                }
                return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
            }

            /* Check if this is a normal character key */
            if (vkCode >= 0x30 && vkCode <= 0x39) {
                /* Digits */
                process_character_input(vkCode - 0x30 + '0');
            } else if (vkCode >= 0x41 && vkCode <= 0x5A) {
                /* Letters */
                process_character_input(vkCode | 0x20);  /* Lowercase */
            } else if (vkCode == VK_OEM_1 || vkCode == VK_OEM_3 || 
                       vkCode == VK_OEM_4 || vkCode == VK_OEM_5 ||
                       vkCode == VK_OEM_6 || vkCode == VK_OEM_7) {
                /* Punctuation */
                process_character_input((char)vkCode);
            }

            /* Inline mode: process buffer automatically */
            if (catkey_current_mode == CATKEY_INPUT_INLINE) {
                process_inline_buffer();
            }
        }
    }

    return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
}

/*
 * Mouse hook (prevent stealing mouse for other operations)
 */
LRESULT CALLBACK catkey_mouse_hook_proc(int nCode, WPARAM wParam, LPARAM lParam) {
    /* We don't intercept mouse events for typing, but we need to know about
     * clicks that could reset our buffer (like clicking outside text field) */
    if (nCode >= 0) {
        switch (wParam) {
            case WM_LBUTTONDOWN:
            case WM_RBUTTONDOWN:
                /* Clicking could interrupt typing - reset buffer */
                catkey_reset_input_buffer();
                break;
        }
    }

    return CallNextHookEx(g_hMouseHook, nCode, wParam, lParam);
}

/*
 * Process character input (add to buffer)
 */
static void process_character_input(char ch) {
    if (g_input_length >= CATKEY_INPUT_BUFFER_SIZE - 1) {
        catkey_reset_input_buffer();
    }

    g_input_buffer[g_input_length++] = ch;
    g_input_buffer[g_input_length] = '\0';

    catkey_log(CATKEY_LOG_DEBUG, "Char '%c' added to buffer: %s", ch, g_input_buffer);
}

/*
 * Process inline buffer - convert automatically as you type
 */
static void process_inline_buffer(void) {
    if (catkey_current_mode != CATKEY_INPUT_INLINE) {
        return;
    }

    if (g_input_length == 0) {
        return;
    }

    /* Convert word using current input method */
    char output[CATKEY_OUTPUT_BUFFER_SIZE];
    int method = (catkey_im_mode == CATKEY_MODE_VNI) ? CATKEY_METHOD_VNI : CATKEY_METHOD_TEIP;

    int out_len = catkey_convert_word(g_input_buffer, output, CATKEY_OUTPUT_BUFFER_SIZE, method);

    if (out_len > 0) {
        catkey_log(CATKEY_LOG_DEBUG, "Inline conversion: %s → %s", g_input_buffer, output);

        /* For inline mode, we'd send the converted text... */
        /* (Actually, inline mode on Windows means raw input, so we don't do this) */
    }
}

/*
 * Process input buffer and commit (for backspace method)
 */
void catkey_process_input_buffer(void) {
    if (g_input_length == 0) {
        return;
    }

    /* Convert the entire buffer */
    char output[CATKEY_OUTPUT_BUFFER_SIZE];
    int method = (catkey_im_mode == CATKEY_MODE_VNI) ? CATKEY_METHOD_VNI : CATKEY_METHOD_TEIP;

    int out_len = catkey_convert_word(g_input_buffer, output, CATKEY_OUTPUT_BUFFER_SIZE, method);

    if (out_len > 0) {
        catkey_log(CATKEY_LOG_INFO, "Backspace method commit: %s → %s", g_input_buffer, output);

        /* Here you'd send the output to the application */
        /* For now, just log it */
    }

    catkey_reset_input_buffer();
}

/*
 * Reset input buffer
 */
void catkey_reset_input_buffer(void) {
    g_input_length = 0;
    g_input_buffer[0] = '\0';
}

/*
 * Check if key is a terminator (space, enter, etc.)
 */
static int is_terminator_key(DWORD vkCode) {
    return (vkCode == VK_SPACE || vkCode == VK_RETURN ||
            vkCode == VK_TAB || vkCode == VK_ESCAPE);
}

/*
 * Check if a character is a Vietnamese vowel
 */
static int is_vietnamese_char(char ch) {
    static const char *vowels = "aăâeêioôơuưy";
    const char *p = strchr(vowels, tolower(ch));
    return (p != NULL);
}

#endif /* _WIN32 */