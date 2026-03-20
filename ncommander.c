#define _GNU_SOURCE
#include <ncurses.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <fcntl.h>

#define MAX_FILES 1000

typedef struct {
    char path[512];           
    char files[MAX_FILES][256];
    int marked[MAX_FILES]; 
    int count, selected, top_file;
    char filter[64];
    WINDOW *win;
} Panel;

int current_theme = 0;

// Helper: Check if item is a directory
int is_dir(const char *parent, const char *name) {
    struct stat st;
    char full[1024];
    snprintf(full, sizeof(full), "%s/%s", parent, name);
    return (stat(full, &st) == 0 && S_ISDIR(st.st_mode));
}

// UI: About Dialog (Key: A)
void about_dialog() {
    int h = 8, w = 45;
    WINDOW *dlg = newwin(h, w, (LINES - h) / 2, (COLS - w) / 2);
    wbkgd(dlg, COLOR_PAIR(2)); box(dlg, 0, 0);
    mvwprintw(dlg, 1, (w - 18) / 2, "nCommander v1.00");
    mvwhline(dlg, 2, 1, ACS_HLINE, w - 2);
    mvwprintw(dlg, 3, (w - 17) / 2, "Developed by:");
    mvwprintw(dlg, 5, (w - 13) / 2, "Olympia Papaioannou");
    mvwprintw(dlg, 6, (w - 20) / 2, "papaioannou.olia@gmail.com");
    wrefresh(dlg);
    wgetch(dlg);
    delwin(dlg);
}

// UI: Color themes
void apply_theme(int id) {
    switch (id) {
        case 0: init_pair(1, COLOR_WHITE, COLOR_BLUE); init_pair(2, COLOR_BLACK, COLOR_CYAN); init_pair(3, COLOR_YELLOW, COLOR_BLUE); break;
        case 1: init_pair(1, COLOR_WHITE, COLOR_BLACK); init_pair(2, COLOR_BLACK, COLOR_WHITE); init_pair(3, COLOR_CYAN, COLOR_BLACK); break;
        case 2: init_pair(1, COLOR_GREEN, COLOR_BLACK); init_pair(2, COLOR_BLACK, COLOR_GREEN); init_pair(3, COLOR_WHITE, COLOR_BLACK); break;
    }
    init_pair(4, COLOR_WHITE, COLOR_RED); 
}

int compare_name(const void *a, const void *b) { return strcasecmp((const char *)a, (const char *)b); }

// UI: File Viewer (F3)
void text_viewer(const char *p, const char *n) {
    char full[1024]; snprintf(full, sizeof(full), "%s/%s", p, n);
    FILE *f = fopen(full, "r"); if (!f) return;
    char lines[1000][256]; int cnt = 0;
    while (cnt < 1000 && fgets(lines[cnt], 256, f)) cnt++;
    fclose(f);
    WINDOW *v = newwin(LINES, COLS, 0, 0); wbkgd(v, COLOR_PAIR(1)); keypad(v, TRUE);
    int off = 0, ch = 0;
    while (ch != 'q' && ch != 27) {
        werase(v); box(v, 0, 0); mvwprintw(v, 0, 2, " View: %s (q/ESC to exit) ", n);
        for (int i = 0; i < LINES - 2 && i + off < cnt; i++) mvwprintw(v, i + 1, 2, "%.*s", COLS - 4, lines[i + off]);
        wrefresh(v); ch = wgetch(v);
        if (ch == KEY_DOWN && off < cnt - (LINES - 2)) off++;
        if (ch == KEY_UP && off > 0) off--;
    }
    delwin(v);
}

void get_input(const char *prompt, char *buffer) {
    WINDOW *in = newwin(5, 50, (LINES - 5) / 2, (COLS - 50) / 2);
    wbkgd(in, COLOR_PAIR(2)); box(in, 0, 0);
    mvwprintw(in, 1, 2, "%s", prompt);
    echo(); curs_set(1); mvwgetnstr(in, 2, 2, buffer, 63); noecho(); curs_set(0);
    delwin(in);
}

int confirm_dialog(const char *msg) {
    WINDOW *dlg = newwin(5, 50, (LINES - 5) / 2, (COLS - 50) / 2);
    wbkgd(dlg, COLOR_PAIR(2)); box(dlg, 0, 0);
    mvwprintw(dlg, 1, (50 - (int)strlen(msg)) / 2, "%s", msg);
    mvwprintw(dlg, 3, 15, "(Y)es / (n)o"); wrefresh(dlg);
    int ch = wgetch(dlg); delwin(dlg);
    return (ch == 'y' || ch == 'Y' || ch == 10);
}

void refresh_panel(Panel *p) {
    DIR *d = opendir(p->path); if (!d) return;
    p->count = 0; struct dirent *e;
    while ((e = readdir(d)) && p->count < MAX_FILES) {
        if (strcmp(e->d_name, ".") == 0) continue;
        if (p->filter[0] == '\0' || strcasestr(e->d_name, p->filter)) {
            snprintf(p->files[p->count], 256, "%s", e->d_name);
            p->count++;
        }
    }
    closedir(d); qsort(p->files, p->count, 256, compare_name);
    if (p->selected >= p->count) p->selected = (p->count > 0) ? p->count - 1 : 0;
}

void draw_panel(Panel *p, int act) {
    werase(p->win); wbkgd(p->win, COLOR_PAIR(1)); box(p->win, 0, 0);
    mvwprintw(p->win, 0, 2, " %s ", p->path);
    int max_y = LINES - 4;
    if (p->selected < p->top_file) p->top_file = p->selected;
    if (p->selected >= p->top_file + max_y) p->top_file = p->selected - max_y + 1;
    for (int i = 0; i < max_y && (i + p->top_file) < p->count; i++) {
        int idx = i + p->top_file;
        if (act && idx == p->selected) wattron(p->win, A_REVERSE);
        if (p->marked[idx]) wattron(p->win, A_BOLD | COLOR_PAIR(2));
        if (is_dir(p->path, p->files[idx])) wattron(p->win, COLOR_PAIR(3));
        mvwprintw(p->win, i + 1, 2, " %c %-28.28s ", p->marked[idx] ? '*' : ' ', p->files[idx]);
        wattroff(p->win, A_REVERSE | A_BOLD | COLOR_PAIR(2) | COLOR_PAIR(3));
    }
    wrefresh(p->win);
}

int main() {
    // 1. Terminal Startup Banner
    printf("\nnCommander version 1.00\n");
    printf("Developed by Olympia Papaioannou\n");
    printf("Contact: papaioannou.olia@gmail.com\n\n");
    printf("Initializing app.. please wait.\n");
    sleep(1); // Short pause to read the info

    initscr(); start_color(); noecho(); curs_set(0); keypad(stdscr, TRUE);
    timeout(100); apply_theme(0);
    Panel l, r; memset(&l, 0, sizeof(Panel)); memset(&r, 0, sizeof(Panel));
    if (!getcwd(l.path, 511)) { strcpy(l.path, "."); }
    if (!getcwd(r.path, 511)) { strcpy(r.path, "."); }
    l.win = newwin(LINES - 2, COLS / 2, 0, 0); r.win = newwin(LINES - 2, COLS / 2, 0, COLS / 2);
    Panel *a = &l, *o = &r;

    while (1) {
        refresh_panel(&l); refresh_panel(&r); draw_panel(&l, a == &l); draw_panel(&r, a == &r);
        struct statfs ds; long long f_gb = 0;
        if (statfs(a->path, &ds) == 0) f_gb = (ds.f_bfree * ds.f_bsize) / (1024 * 1024 * 1024);
        attron(COLOR_PAIR(2)); mvhline(LINES - 2, 0, ' ', COLS);
        mvprintw(LINES - 2, 1, "Free:%lldGB | Space:Mark | S:Search | T:Theme | A:About", f_gb); attroff(COLOR_PAIR(2));
        attron(COLOR_PAIR(4)); mvhline(LINES - 1, 0, ' ', COLS);
        mvprintw(LINES - 1, 0, "F1:Hm F2:Chm F3:Vw F4:Ed F5:Cp F6:Mv F7:MkD F8:Del F9:Zip F10:Unz Q:Ex");
        attroff(COLOR_PAIR(4)); refresh(); 

        int ch = getch(); if (ch == 'q' || ch == 'Q') break; if (ch == ERR) continue;
        char buf[256], cmd[2048]; char *cur = a->files[a->selected];

        switch(ch) {
            case 'a': case 'A': about_dialog(); break; // New About Info
            case ' ': if (a->count > 0) a->marked[a->selected] = !a->marked[a->selected]; break;
            case KEY_UP: if (a->selected > 0) a->selected--; break;
            case KEY_DOWN: if (a->selected < a->count - 1) a->selected++; break;
            case KEY_LEFT: a = &l; o = &r; break;
            case KEY_RIGHT: a = &r; o = &l; break;
            case '\t': a = (a == &l) ? &r : &l; o = (a == &l) ? &r : &l; break;
            case 't': case 'T': current_theme = (current_theme + 1) % 3; apply_theme(current_theme); clear(); break;
            case 's': case 'S': case '/': get_input("Filter:", a->filter); a->selected = 0; break;
            case KEY_BACKSPACE: case 127: case 8:
                if (chdir(a->path) == 0 && chdir("..") == 0) {
                    if (getcwd(a->path, 511)) { a->selected = 0; a->filter[0] = '\0'; memset(a->marked, 0, sizeof(a->marked)); }
                } break;
            case KEY_F(1): { char *h = getenv("HOME"); if(h) { strncpy(a->path, h, 511); a->selected = 0; a->filter[0] = '\0'; } } break;
            case KEY_F(2): if (a->count > 0) { get_input("Chm (e.g. 755):", buf); if (buf[0] != '\0') { snprintf(cmd, 2048, "chmod %s '%s/%s' > /dev/null 2>&1", buf, a->path, cur); if(system(cmd)){} } } break;
            case KEY_F(3): if (a->count > 0 && !is_dir(a->path, cur)) text_viewer(a->path, cur); break;
            case KEY_F(4): if (a->count > 0 && !is_dir(a->path, cur)) { def_prog_mode(); endwin(); snprintf(cmd, 2048, "nano '%s/%s'", a->path, cur); if(system(cmd)){} reset_prog_mode(); refresh(); } break;
            case KEY_F(5): {
                int m = 0; for(int i=0; i<a->count; i++) if(a->marked[i]) m = 1;
                if (confirm_dialog(m ? "Copy marked?" : "Copy selected?")) {
                    if (m) { for(int i=0; i<a->count; i++) if(a->marked[i]) { snprintf(cmd, 2048, "cp -rf '%s/%s' '%s/' > /dev/null 2>&1", a->path, a->files[i], o->path); if(system(cmd)){} a->marked[i] = 0; } }
                    else { snprintf(cmd, 2048, "cp -rf '%s/%s' '%s/' > /dev/null 2>&1", a->path, cur, o->path); if(system(cmd)){} }
                }
            } break;
            case KEY_F(6): {
                int m = 0; for(int i=0; i<a->count; i++) if(a->marked[i]) m = 1;
                if (confirm_dialog(m ? "Move marked?" : "Move selected?")) {
                    if (m) { for(int i=0; i<a->count; i++) if(a->marked[i]) { snprintf(cmd, 2048, "mv -f '%s/%s' '%s/' > /dev/null 2>&1", a->path, a->files[i], o->path); if(system(cmd)){} a->marked[i] = 0; } }
                    else { snprintf(cmd, 2048, "mv -f '%s/%s' '%s/' > /dev/null 2>&1", a->path, cur, o->path); if(system(cmd)){} }
                }
            } break;
            case KEY_F(7): get_input("New Folder Name:", buf); if (buf[0] != '\0') { char nf[1024]; snprintf(nf, 1024, "%s/%s", a->path, buf); if (mkdir(nf, 0777) == 0) { if (chdir(nf) == 0) { if (getcwd(a->path, 511)) { a->selected = 0; a->filter[0] = '\0'; } } } } break;
            case KEY_F(8): {
                int m = 0; for(int i=0; i<a->count; i++) if(a->marked[i]) m = 1;
                if (confirm_dialog(m ? "Delete marked?" : "Delete selected?")) {
                    if (m) { for(int i=0; i<a->count; i++) if(a->marked[i]) { snprintf(cmd, 2048, "rm -rf '%s/%s' > /dev/null 2>&1", a->path, a->files[i]); if(system(cmd)){} a->marked[i] = 0; } }
                    else { snprintf(cmd, 2048, "rm -rf '%s/%s' > /dev/null 2>&1", a->path, cur); if(system(cmd)){} }
                }
            } break;
            case KEY_F(9): {
                int m = 0; for(int i=0; i<a->count; i++) if(a->marked[i]) m = 1;
                get_input("Zip name:", buf); if (buf[0] != '\0') {
                    if (m) { for(int i=0; i<a->count; i++) if(a->marked[i]) { snprintf(cmd, 2048, "zip -r '%s/%s' '%s/%s' > /dev/null 2>&1", a->path, buf, a->path, a->files[i]); if(system(cmd)){} a->marked[i] = 0; } }
                    else { snprintf(cmd, 2048, "zip -r '%s/%s' '%s/%s' > /dev/null 2>&1", a->path, buf, a->path, cur); if(system(cmd)){} }
                }
            } break;
            case KEY_F(10): if (a->count > 0 && strstr(cur, ".zip")) { if(confirm_dialog("Unzip here?")) { snprintf(cmd, 2048, "unzip -o '%s/%s' -d '%s/' > /dev/null 2>&1", a->path, cur, a->path); if(system(cmd)){} } } break;
            case 10: if (a->count > 0 && is_dir(a->path, cur)) { char nx[1024]; snprintf(nx, 1024, "%s/%s", a->path, cur); if(chdir(nx) == 0) { if(getcwd(a->path, 511)) { a->selected = 0; a->filter[0] = '\0'; memset(a->marked, 0, sizeof(a->marked)); } } } break;
        }
    }
    endwin(); return 0;
}
