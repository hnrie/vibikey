/*
 * CatKey - Vietnamese Input Method
 * Input Method Selection Dialog - Linux Implementation
 * 
 * Uses ncurses for terminal-based dialog
 */

#if defined(CATKEY_LINUX)

#include "../catkey_input_method_dialog.h"
#include "../config.h"
#include <ncurses.h>
#include <string.h>

/*
 * Show the method selection dialog
 * 
 * @return: selected method type, or -1 if cancelled
 */
int catkey_show_method_dialog(void) {
    catkey_method_desc_t methods[CATKEY_METHOD_MAX];
    int count = catkey_get_method_list(methods, CATKEY_METHOD_MAX);

    if (count == 0) {
        return -1;
    }

    /* Initialize ncurses */
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    int selected = 0;
    int current = 0;
    int done = 0;

    while (!done) {
        clear();
        mvprintw(1, 2, "CatKey - Select Input Method");
        mvprintw(2, 2, "Available methods are shown normally, unavailable are greyed (x)");

        /* Draw method list */
        for (int i = 0; i < count; i++) {
            int line = 4 + i;
            const char *status = methods[i].is_available ? " " : "[x]";
            const char *avail = methods[i].is_available ? 
                "" : " (Unavailable for your platform)";

            if (i == current) {
                attron(A_REVERSE);
            }

            if (!methods[i].is_available) {
                attron(COLOR_PAIR(1));  /* Grey/dim color */
            }

            mvprintw(line, 2, "%s %d. %s - %s%s",
                status,
                i + 1,
                methods[i].name,
                methods[i].description,
                avail
            );

            if (!methods[i].is_available) {
                attroff(COLOR_PAIR(1));
            }

            if (i == current) {
                attroff(A_REVERSE);
            }
        }

        mvprintw(4 + count + 2, 2, "↑/↓: Navigate  Enter: Select  q: Quit");

        int ch = getch();
        switch (ch) {
            case KEY_UP:
                if (current > 0) current--;
                break;
            case KEY_DOWN:
                if (current < count - 1) current++;
                break;
            case '\n':
            case KEY_ENTER: {
                if (methods[current].is_available) {
                    selected = methods[current].type;
                    done = 1;
                } else {
                    /* Show unavailable message */
                    mvprintw(4 + count + 4, 2, 
                        "This input method is unavailable for your platform. Press any key...");
                    getch();
                }
                break;
            }
            case 'q':
            case 'Q':
                selected = -1;
                done = 1;
                break;
        }
    }

    endwin();
    return selected;
}

#endif /* CATKEY_LINUX */