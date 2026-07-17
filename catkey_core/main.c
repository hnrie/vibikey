/*
 * CatKey - Vietnamese Input Method
 * Main entry point
 * 
 * Cross-platform entry for Windows and Linux
 * Handles: initialization, method selection, daemon startup
 */

#include "config.h"
#include "catkey_input_method_dialog.h"
#include <stdio.h>
#include <string.h>

#if defined(CATKEY_WINDOWS)
#include "windows/catkey_windows_hook.h"
#include <windows.h>
#elif defined(CATKEY_LINUX)
#include "linux/catkey_linux_daemon.h"
#include <unistd.h>
#include <signal.h>
#endif

/* Global config */
int catkey_backend = CATKEY_BACKEND_PLATFORM;
int catkey_im_mode = CATKEY_MODE_TEIP;

/* Signal handler for clean shutdown */
#if defined(CATKEY_LINUX)
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
        "CatKey - Vietnamese Input Method v%s\n"
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
        CATKEY_VERSION_STRING, prog
    );
}

/*
 * Parse command line arguments
 */
static int parse_args(int argc, char *argv[], catkey_method_type_t *method, int *show_dialog) {
    *method = CATKEY_METHOD_BACKSPACE;
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
            catkey_im_mode = CATKEY_MODE_TEIP;
        }
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--vni") == 0) {
            catkey_im_mode = CATKEY_MODE_VNI;
        }
        else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--method") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: --method requires a value\n");
                return -1;
            }
            i++;
            if (strcmp(argv[i], "backspace") == 0) {
                *method = CATKEY_METHOD_BACKSPACE;
            } else if (strcmp(argv[i], "inline") == 0) {
                *method = CATKEY_METHOD_INLINE;
            } else if (strcmp(argv[i], "ibus") == 0) {
                *method = CATKEY_METHOD_IBUS;
            } else if (strcmp(argv[i], "fcitx") == 0) {
                *method = CATKEY_METHOD_FCITX;
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

#if defined(CATKEY_WINDOWS)
/*
 * Windows entry point (WinMain for GUI, main for console)
 */
int main(int argc, char *argv[]) {
    catkey_method_type_t method;
    int show_dialog;
    int ret = parse_args(argc, argv, &method, &show_dialog);
    if (ret != 0) return ret;

    catkey_log(CATKEY_LOG_INFO, "CatKey v%s starting on Windows", CATKEY_VERSION_STRING);

    /* Initialize config */
    catkey_log_set_callback(NULL);  /* Use default stderr logging */

    /* Show method selection dialog if requested */
    if (show_dialog) {
        catkey_log(CATKEY_LOG_INFO, "Showing method selection dialog");
        int selected = catkey_show_method_dialog();
        if (selected < 0) {
            catkey_log(CATKEY_LOG_INFO, "Dialog cancelled, exiting");
            return 0;
        }
        method = (catkey_method_type_t)selected;
    }

    /* Check if method is available */
    if (!catkey_method_is_available(method)) {
        fprintf(stderr, "Error: Method %d is not available on this platform\n", method);
        return 1;
    }

    /* Initialize Windows hooks */
    if (catkey_windows_init() != 0) {
        catkey_log(CATKEY_LOG_ERROR, "Failed to initialize Windows hooks");
        return 1;
    }

    /* Set input mode */
    catkey_input_mode_t mode;
    switch (method) {
        case CATKEY_METHOD_INLINE: mode = CATKEY_INPUT_INLINE; break;
        default: mode = CATKEY_INPUT_BACKSPACE; break;
    }
    catkey_windows_set_mode(mode);

    catkey_log(CATKEY_LOG_INFO, "CatKey running. Press hotkeys to switch modes.");

    /* Windows message loop (required for hooks to work) */
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    /* Cleanup */
    catkey_windows_cleanup();
    catkey_log(CATKEY_LOG_INFO, "CatKey stopped");

    return 0;
}

#elif defined(CATKEY_LINUX)
/*
 * Linux entry point
 */
int main(int argc, char *argv[]) {
    catkey_method_type_t method;
    int show_dialog;
    int ret = parse_args(argc, argv, &method, &show_dialog);
    if (ret != 0) return ret;

    catkey_log(CATKEY_LOG_INFO, "CatKey v%s starting on Linux", CATKEY_VERSION_STRING);

    /* Show method selection dialog if requested */
    if (show_dialog) {
        catkey_log(CATKEY_LOG_INFO, "Showing method selection dialog");
        int selected = catkey_show_method_dialog();
        if (selected < 0) {
            catkey_log(CATKEY_LOG_INFO, "Dialog cancelled, exiting");
            return 0;
        }
        method = (catkey_method_type_t)selected;
    }

    /* Check if method is available */
    if (!catkey_method_is_available(method)) {
        fprintf(stderr, "Error: Method %d is not available on this platform\n", method);
        return 1;
    }

    /* Initialize Linux daemon */
    if (catkey_linux_init() != 0) {
        catkey_log(CATKEY_LOG_ERROR, "Failed to initialize Linux daemon");
        return 1;
    }

    /* Set input mode */
    catkey_linux_mode_t mode;
    switch (method) {
        case CATKEY_METHOD_INLINE: mode = CATKEY_LINUX_INLINE; break;
        default: mode = CATKEY_LINUX_BACKSPACE; break;
    }
    catkey_linux_set_mode(mode);

    /* Set up signal handlers */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    catkey_log(CATKEY_LOG_INFO, "CatKey running. Press Ctrl+C to stop.");

    /* Main loop - in a real implementation, this would be:
     * - X11 event loop for keyboard filtering
     * - Or IBus/Fcitx module initialization
     */
    while (s_running) {
        /* TODO: X11 event loop or IBus/Fcitx integration */
        usleep(100000);  /* 100ms sleep */
    }

    /* Cleanup */
    catkey_linux_cleanup();
    catkey_log(CATKEY_LOG_INFO, "CatKey stopped");

    return 0;
}
#endif