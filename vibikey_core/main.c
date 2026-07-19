/*
 * VibiKey - Vietnamese Input Method
 * Main entry point
 * 
 * Cross-platform entry for Windows and Linux
 * Handles: initialization, method selection, daemon startup
 */

#define _DEFAULT_SOURCE
#include "config.h"
#include "vibikey_input_method_dialog.h"
#include <stdio.h>
#include <string.h>

#if defined(VIBIKEY_WINDOWS)
#include "windows/vibikey_windows_hook.h"
#include <windows.h>
#elif defined(VIBIKEY_LINUX)
#include "linux/vibikey_linux_daemon.h"
#include <unistd.h>
#include <signal.h>
#endif

/* Global config */
int vibikey_backend = VIBIKEY_BACKEND_PLATFORM;
int vibikey_im_mode = VIBIKEY_MODE_TEIP;

/* Signal handler for clean shutdown */
#if defined(VIBIKEY_LINUX)
static volatile int s_running = 1;
static void signal_handler(int sig) {
    (void)sig;
    s_running = 0;
}
#endif

/*
 * Print usage
 */
static void print_usage(const char *prog) {
    fprintf(stderr,
        "VibiKey - Vietnamese Input Method v%s\n"
        "Usage: %s [options]\n"
        "\n"
        "Options:\n"
        "  -m, --method <name>   Input method (backspace, inline)\n"
        "  -t, --teip            Use TEIP method (default)\n"
        "  -v, --vni             Use VNI method\n"
        "  -d, --dialog          Show method selection dialog\n"
        "  -h, --help            Show this help\n"
        "\n"
        "Methods:\n"
        "  backspace  Type to buffer, press Backspace to commit\n"
        "  inline     Auto-convert while typing\n"
        "  ibus       IBus integration (Linux only)\n"
        "  fcitx      Fcitx integration (Linux only)\n",
        VIBIKEY_VERSION_STRING, prog
    );
}

/*
 * Parse command line arguments
 */
static int parse_args(int argc, char *argv[], vibikey_method_type_t *method, int *show_dialog) {
    *method = VIBIKEY_METHOD_BACKSPACE;
    *show_dialog = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 1;
        }
        else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--dialog") == 0) {
            *show_dialog = 1;
        }
        else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--teip") == 0) {
            vibikey_im_mode = VIBIKEY_MODE_TEIP;
        }
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--vni") == 0) {
            vibikey_im_mode = VIBIKEY_MODE_VNI;
        }
        else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--method") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: --method requires a value\n");
                return -1;
            }
            i++;
            if (strcmp(argv[i], "backspace") == 0) {
                *method = VIBIKEY_METHOD_BACKSPACE;
            } else if (strcmp(argv[i], "inline") == 0) {
                *method = VIBIKEY_METHOD_INLINE;
            } else if (strcmp(argv[i], "ibus") == 0) {
                *method = VIBIKEY_METHOD_IBUS;
            } else if (strcmp(argv[i], "fcitx") == 0) {
                *method = VIBIKEY_METHOD_FCITX;
            } else {
                fprintf(stderr, "Error: Unknown method '%s'\n", argv[i]);
                return -1;
            }
        }
        else {
            fprintf(stderr, "Error: Unknown option '%s'\n", argv[i]);
            print_usage(argv[0]);
            return -1;
        }
    }

    return 0;
}

#if defined(VIBIKEY_WINDOWS)
/*
 * Windows entry point (WinMain for GUI, main for console)
 */
int main(int argc, char *argv[]) {
    vibikey_method_type_t method;
    int show_dialog;
    int ret = parse_args(argc, argv, &method, &show_dialog);
    if (ret != 0) return ret;

    vibikey_log(VIBIKEY_LOG_INFO, "VibiKey v%s starting on Windows", VIBIKEY_VERSION_STRING);

    /* Initialize config */
    vibikey_log_set_callback(NULL);  /* Use default stderr logging */

    /* Show method selection dialog if requested */
    if (show_dialog) {
        vibikey_log(VIBIKEY_LOG_INFO, "Showing method selection dialog");
        int selected = vibikey_show_method_dialog();
        if (selected < 0) {
            vibikey_log(VIBIKEY_LOG_INFO, "Dialog cancelled, exiting");
            return 0;
        }
        method = (vibikey_method_type_t)selected;
    }

    /* Check if method is available */
    if (!vibikey_method_is_available(method)) {
        fprintf(stderr, "Error: Method %d is not available on this platform\n", method);
        return 1;
    }

    /* Initialize Windows hooks */
    if (vibikey_windows_init() != 0) {
        vibikey_log(VIBIKEY_LOG_ERROR, "Failed to initialize Windows hooks");
        return 1;
    }

    /* Set input mode */
    vibikey_input_mode_t mode;
    switch (method) {
        case VIBIKEY_METHOD_INLINE: mode = VIBIKEY_INPUT_INLINE; break;
        default: mode = VIBIKEY_INPUT_BACKSPACE; break;
    }
    vibikey_windows_set_mode(mode);

    vibikey_log(VIBIKEY_LOG_INFO, "VibiKey running. Press hotkeys to switch modes.");

    /* Windows message loop (required for hooks to work) */
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    /* Cleanup */
    vibikey_windows_cleanup();
    vibikey_log(VIBIKEY_LOG_INFO, "VibiKey stopped");

    return 0;
}

#elif defined(VIBIKEY_LINUX)
/*
 * Linux entry point
 */
int main(int argc, char *argv[]) {
    vibikey_method_type_t method;
    int show_dialog;
    int ret = parse_args(argc, argv, &method, &show_dialog);
    if (ret != 0) return ret;

    vibikey_log(VIBIKEY_LOG_INFO, "VibiKey v%s starting on Linux", VIBIKEY_VERSION_STRING);

    /* Show method selection dialog if requested */
    if (show_dialog) {
        vibikey_log(VIBIKEY_LOG_INFO, "Showing method selection dialog");
        int selected = vibikey_show_method_dialog();
        if (selected < 0) {
            vibikey_log(VIBIKEY_LOG_INFO, "Dialog cancelled, exiting");
            return 0;
        }
        method = (vibikey_method_type_t)selected;
    }

    /* Check if method is available */
    if (!vibikey_method_is_available(method)) {
        fprintf(stderr, "Error: Method %d is not available on this platform\n", method);
        return 1;
    }

    /* Initialize Linux daemon */
    if (vibikey_linux_init() != 0) {
        vibikey_log(VIBIKEY_LOG_ERROR, "Failed to initialize Linux daemon");
        return 1;
    }

    /* Set input mode */
    vibikey_linux_mode_t mode;
    switch (method) {
        case VIBIKEY_METHOD_INLINE: mode = VIBIKEY_LINUX_INLINE; break;
        default: mode = VIBIKEY_LINUX_BACKSPACE; break;
    }
    vibikey_linux_set_mode(mode);

    /* Set up signal handlers */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    vibikey_log(VIBIKEY_LOG_INFO, "VibiKey running. Press Ctrl+C to stop.");

    /* Main loop - in a real implementation, this would be:
     * - X11 event loop for keyboard filtering
     * - Or IBus/Fcitx module initialization
     */
    while (s_running) {
        /* TODO: X11 event loop or IBus/Fcitx integration */
        usleep(100000);  /* 100ms sleep */
    }

    /* Cleanup */
    vibikey_linux_cleanup();
    vibikey_log(VIBIKEY_LOG_INFO, "VibiKey stopped");

    return 0;
}
#endif