/*
 * CatKey - Vietnamese Input Method
 * Windows Dialog Resource Definitions
 */

#if defined(CATKEY_WINDOWS)

#define IDC_METHOD_LIST    1001
#define IDC_STATUS_TEXT    1002
#define IDD_METHOD_DIALOG  1003
#define IDOK                1
#define IDCANCEL            2

/* Dialog template (for use with CreateDialogIndirect or DialogBoxParam) */
static const DLGTEMPLATE g_method_dialog_template = {
    sizeof(DLGTEMPLATE),
    0,  /* style */
    2,  /* item count */
    10, 10,  /* x, y */
    300, 200,  /* width, height */
    0,  /* menu */
    0,  /* class */
    L"CatKey Input Method",  /* title */
    8,  /* font size */
    L"MS Shell Dlg"  /* font */
};

#endif /* CATKEY_WINDOWS */