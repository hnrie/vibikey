/*
 * CatKey - Vietnamese Input Method
 * Input Method Selection Dialog - Windows Implementation (Win32)
 * 
 * Uses CreateWindowEx for programmatic UI (no .rc resource file needed)
 */

#if defined(CATKEY_WINDOWS)

#include "../catkey_input_method_dialog.h"
#include "../config.h"
#include <windows.h>

/* Window class name */
#define CATKEY_DIALOG_CLASS L"CatKeyMethodDialog"

/* Control IDs */
#define IDC_LISTBOX    1001
#define IDC_LABEL      1002
#define IDC_OK         1003
#define IDC_CANCEL     1004

/* Dialog state */
static catkey_method_desc_t *s_methods = NULL;
static int s_method_count = 0;
static int s_selected = -1;
static HWND s_hwndDialog = NULL;
static HWND s_hwndList = NULL;
static HWND s_hwndLabel = NULL;
static HWND s_hwndOK = NULL;
static HWND s_hwndCancel = NULL;

/* Window procedure */
static LRESULT CALLBACK dialog_wndproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            /* Create label */
            s_hwndLabel = CreateWindowExW(
                0, L"STATIC",
                L"Select Input Method (greyed = unavailable on your platform)",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                10, 10, 380, 20,
                hwnd, (HMENU)IDC_LABEL, GetModuleHandle(NULL), NULL
            );

            /* Create listbox */
            s_hwndList = CreateWindowExW(
                WS_EX_CLIENTEDGE,
                L"LISTBOX",
                NULL,
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_HASSTRINGS,
                10, 35, 380, 180,
                hwnd, (HMENU)IDC_LISTBOX, GetModuleHandle(NULL), NULL
            );

            /* Populate listbox */
            for (int i = 0; i < s_method_count; i++) {
                int idx = (int)SendMessageW(s_hwndList, LB_ADDSTRING, 0,
                    (LPARAM)s_methods[i].name);

                if (idx != LB_ERR) {
                    /* Store method index in item data */
                    SendMessageW(s_hwndList, LB_SETITEMDATA, idx, (LPARAM)i);

                    /* Grey out unavailable methods by disabling their visual state */
                    if (!s_methods[i].is_available) {
                        /* We can't grey individual items easily, but we append
                         * " [unavailable]" to make it clear */
                        wchar_t buf[128];
                        wsprintfW(buf, L"%hs [unavailable]", s_methods[i].name);
                        SendMessageW(s_hwndList, LB_DELETESTRING, idx, 0);
                        idx = (int)SendMessageW(s_hwndList, LB_ADDSTRING, 0, (LPARAM)buf);
                        SendMessageW(s_hwndList, LB_SETITEMDATA, idx, (LPARAM)i);
                    }
                }
            }

            /* Select first available method */
            for (int i = 0; i < s_method_count; i++) {
                if (s_methods[i].is_available) {
                    SendMessageW(s_hwndList, LB_SETCURSEL, i, 0);
                    break;
                }
            }

            /* Create OK button */
            s_hwndOK = CreateWindowExW(
                0, L"BUTTON", L"OK",
                WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                220, 225, 80, 25,
                hwnd, (HMENU)IDC_OK, GetModuleHandle(NULL), NULL
            );

            /* Create Cancel button */
            s_hwndCancel = CreateWindowExW(
                0, L"BUTTON", L"Cancel",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                310, 225, 80, 25,
                hwnd, (HMENU)IDC_CANCEL, GetModuleHandle(NULL), NULL
            );

            return 0;
        }

        case WM_COMMAND: {
            int ctrl_id = LOWORD(wParam);
            int notify = HIWORD(wParam);

            if (ctrl_id == IDC_OK || 
                (ctrl_id == IDC_LISTBOX && notify == LBN_DBLCLK)) {
                int sel = (int)SendMessageW(s_hwndList, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR) {
                    int method_idx = (int)SendMessageW(s_hwndList, LB_GETITEMDATA, sel, 0);
                    if (method_idx >= 0 && method_idx < s_method_count &&
                        s_methods[method_idx].is_available) {
                        s_selected = s_methods[method_idx].type;
                        DestroyWindow(hwnd);
                    } else {
                        MessageBoxW(hwnd,
                            L"This input method is unavailable for your platform.",
                            L"Unavailable", MB_OK | MB_ICONINFORMATION);
                    }
                }
                return 0;
            }

            if (ctrl_id == IDC_CANCEL) {
                s_selected = -1;
                DestroyWindow(hwnd);
                return 0;
            }
            break;
        }

        case WM_DESTROY:
            s_hwndDialog = NULL;
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

/*
 * Show the method selection dialog
 */
int catkey_show_method_dialog(void) {
    catkey_method_desc_t methods[CATKEY_METHOD_MAX];
    int count = catkey_get_method_list(methods, CATKEY_METHOD_MAX);

    if (count == 0) {
        return -1;
    }

    s_methods = methods;
    s_method_count = count;
    s_selected = -1;

    /* Register window class */
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = dialog_wndproc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = CATKEY_DIALOG_CLASS;

    if (!RegisterClassExW(&wc)) {
        return -1;
    }

    /* Calculate centered position */
    int screen_w = GetSystemMetrics(SM_CXSCREEN);
    int screen_h = GetSystemMetrics(SM_CYSCREEN);
    int dlg_w = 410;
    int dlg_h = 290;
    int dlg_x = (screen_w - dlg_w) / 2;
    int dlg_y = (screen_h - dlg_h) / 2;

    /* Create window */
    s_hwndDialog = CreateWindowExW(
        0,
        CATKEY_DIALOG_CLASS,
        L"CatKey - Select Input Method",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        dlg_x, dlg_y, dlg_w, dlg_h,
        NULL, NULL, GetModuleHandle(NULL), NULL
    );

    if (!s_hwndDialog) {
        UnregisterClassW(CATKEY_DIALOG_CLASS, GetModuleHandle(NULL));
        return -1;
    }

    ShowWindow(s_hwndDialog, SW_SHOW);
    UpdateWindow(s_hwndDialog);

    /* Message loop */
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        if (msg.message == WM_QUIT) {
            break;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    UnregisterClassW(CATKEY_DIALOG_CLASS, GetModuleHandle(NULL));

    return s_selected;
}

#endif /* CATKEY_WINDOWS */