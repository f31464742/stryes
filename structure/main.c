#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#endif

#define OPTIONS 3
#define MAX_QUOTES 10

// Тексты из конфигов
char gfx_border[64];
char gfx_arrow_left[8];
char gfx_arrow_right[8];

char menu_items[OPTIONS][64];
char prompt_continue[128];
char quotes[MAX_QUOTES][256];

// Получение размера терминала
void get_terminal_size(int *width, int *height) {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    *width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    *height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    *width = w.ws_col;
    *height = w.ws_row;
#endif
}

// Очистка экрана
void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// Центрированный вывод строки
void print_centered(const char *text, int width) {
    int len = strlen(text);
    int pad = (width - len) / 2;
    if (pad < 0) pad = 0;
    printf("%*s%s\n", pad, "", text);
}

// Кроссплатформенное чтение клавиш
int get_key() {
#ifdef _WIN32
    int ch = _getch();
    if (ch == 0 || ch == 224) {
        ch = _getch();
        return (ch == 72) ? 'U' : (ch == 80) ? 'D' : 0;
    }
    return (ch == 13) ? '\n' : ch;
#else
    struct termios oldt, newt;
    int ch;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    ch = getchar();
    if (ch == 27 && getchar() == '[') {
        switch (getchar()) {
            case 'A': ch = 'U'; break;
            case 'B': ch = 'D'; break;
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
#endif
}

// Загрузка конфигов
void load_configurations() {
    FILE *lang = fopen("confirmation/language.cfg", "r");
    FILE *gfx = fopen("confirmation/graphics.cfg", "r");

    if (!lang || !gfx) {
        printf("eror cfg\n");
        exit(1);
    }

    fscanf(lang, "menu_start = %[^\n]\n", menu_items[0]);
    fscanf(lang, "menu_settings = %[^\n]\n", menu_items[1]);
    fscanf(lang, "menu_exit = %[^\n]\n", menu_items[2]);
    fscanf(lang, "prompt_continue = %[^\n]\n", prompt_continue);
    for (int i = 0; i < MAX_QUOTES; i++) {
        fscanf(lang, "quote%d = %[^\n]\n", &i, quotes[i]);
    }

    fscanf(gfx, "border = %[^\n]\n", gfx_border);
    fscanf(gfx, "arrow_left = %[^\n]\n", gfx_arrow_left);
    fscanf(gfx, "arrow_right = %[^\n]\n", gfx_arrow_right);

    fclose(lang);
    fclose(gfx);
}

// Меню
int menu_loop() {
    int selection = 0;
    int term_w, term_h;

    while (1) {
        get_terminal_size(&term_w, &term_h);
        clear_screen();

        // Вертикальное центрирование
        int menu_height = OPTIONS;
        int vertical_padding = (term_h - menu_height - 2) / 2;
        for (int i = 0; i < vertical_padding; ++i) printf("\n");

        // Центрированный бордер
        print_centered(gfx_border, term_w);

        for (int i = 0; i < OPTIONS; ++i) {
            char line[128];
            if (i == selection) {
                snprintf(line, sizeof(line), "%s %s %s", gfx_arrow_left, menu_items[i], gfx_arrow_right);
            } else {
                snprintf(line, sizeof(line), "  %s  ", menu_items[i]);
            }
            print_centered(line, term_w);
        }

        print_centered(gfx_border, term_w);

        int key = get_key();
        if (key == 'U') {
            selection = (selection - 1 + OPTIONS) % OPTIONS;
        } else if (key == 'D') {
            selection = (selection + 1) % OPTIONS;
        } else if (key == '\n') {
            return selection;
        }
    }
}

// Сессия "НАЧАТЬ"
void start_session() {
    char input[16];
    int term_w, term_h;

    for (int i = 0; i < MAX_QUOTES; ++i) {
        get_terminal_size(&term_w, &term_h);
        clear_screen();

        for (int j = 0; j < (term_h / 2) - 1; ++j) printf("\n");
        print_centered(gfx_border, term_w);
        print_centered(quotes[i], term_w);
        print_centered(gfx_border, term_w);

        printf("\n%s", prompt_continue);
        fgets(input, sizeof(input), stdin);
        if (input[0] != 'y' && input[0] != 'Y') break;
    }
}

// main
int main() {
    load_configurations();

    while (1) {
        int choice = menu_loop();
        if (choice == 0) {
            start_session();
        } else if (choice == 1) {
            clear_screen();
            int w, h;
            get_terminal_size(&w, &h);
            for (int i = 0; i < h / 2; ++i) printf("\n");
            getchar();
        } else {
            clear_screen();
            int w, h;
            get_terminal_size(&w, &h);
            for (int i = 0; i < h / 2; ++i) printf("\n");
            break;
        }
    }

    return 0;
}
