#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "editor.h"

#define CAPACITY 128

void init_document(Document *doc) {
    doc->capacity = 128;
    doc->line_count = 0;
    doc->modified = 0;
    doc->lines = malloc(sizeof(char*) * doc->capacity);
}

void free_document(Document *doc) {
    for (int i = 0; i < doc->line_count; i++)
        free(doc->lines[i]);
    free(doc->lines);
}

static void add_empty_line(Document *doc) {
    if (doc->line_count >= doc->capacity) {
        doc->capacity *= 2;
        doc->lines = realloc(doc->lines, sizeof(char*) * doc->capacity);
    }
    doc->lines[doc->line_count] = malloc(256);
    doc->lines[doc->line_count][0] = '\0';
    doc->line_count++;
}

void insert_char(Document *doc, Cursor *cur, char ch) {
    char *line = doc->lines[cur->y];
    int len = strlen(line);
    for (int i = len; i > cur->x; i--)
        line[i] = line[i - 1];
    line[cur->x] = ch;
    line[len + 1] = '\0';
    cur->x++;
    doc->modified = 1;
}

void backspace_char(Document *doc, Cursor *cur) {
    if (cur->x == 0) return;
    char *line = doc->lines[cur->y];
    int len = strlen(line);
    for (int i = cur->x - 1; i < len; i++)
        line[i] = line[i + 1];
    cur->x--;
    doc->modified = 1;
}

void split_line(Document *doc, Cursor *cur) {
    char *line = doc->lines[cur->y];
    char *new_line = malloc(256);
    strcpy(new_line, line + cur->x);
    line[cur->x] = '\0';

    if (doc->line_count >= doc->capacity) {
        doc->capacity *= 2;
        doc->lines = realloc(doc->lines, sizeof(char*) * doc->capacity);
    }

    for (int i = doc->line_count; i > cur->y + 1; i--)
        doc->lines[i] = doc->lines[i - 1];

    doc->lines[cur->y + 1] = new_line;
    doc->line_count++;
    cur->y++;
    cur->x = 0;
    doc->modified = 1;
}

int load_file(Document *doc, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        add_empty_line(doc);
        return -1;
    }

    char buf[256];
    while (fgets(buf, sizeof(buf), file)) {
        buf[strcspn(buf, "\n")] = '\0';
        if (doc->line_count >= doc->capacity) {
            doc->capacity *= 2;
            doc->lines = realloc(doc->lines, sizeof(char*) * doc->capacity);
        }
        doc->lines[doc->line_count] = malloc(256);
        strncpy(doc->lines[doc->line_count], buf, 255);
        doc->lines[doc->line_count][255] = '\0';
        doc->line_count++;
    }

    fclose(file);

    if (doc->line_count == 0)
        add_empty_line(doc);

    doc->modified = 0;
    return 0;
}

int save_file(Document *doc, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) return -1;

    for (int i = 0; i < doc->line_count; i++)
        fprintf(file, "%s\n", doc->lines[i]);

    fclose(file);
    doc->modified = 0;
    return 0;
}


// draw the contents of the window. Also have a scroll back support
void draw(Document *doc, Cursor *cur, Mode mode, int cmd_active, CommandBuffer *cmd, const char *msg, int scroll) {
    int h, w;

    getmaxyx(stdscr, h, w);	// get the coords of the current screen window
    clear();			// clear the screen
    box(stdscr, 0, 0);		// create a box window at the origin
 
    for (int i = 0; i < h - 3; i++) { // before the mode section
        int doc_line = scroll + i;
        if (doc_line >= doc->line_count) break;
        mvprintw(1 + i, 1, "%s", doc->lines[doc_line]); // line count at the rigtmost in the mode bar
    }
 
    attron(A_REVERSE);
    mvhline(h - 2, 0, ' ', w);
 
    const char *mode_str = (mode == NORMAL) ? "-- NORMAL --" :
                           (mode == INSERT) ? "-- INSERT --" : "-- COMMAND --";
    mvprintw(h - 2, 2, "%s", mode_str);
 
    char info[128];
    const char *mod_flag = doc->modified ? "[MODIFIED]" : "";
    sprintf(info, "%d lines %s", doc->line_count, mod_flag);
    mvprintw(h - 2, w - strlen(info) - 2, "%s", info);
    attroff(A_REVERSE);
 
    if (cmd_active) {
        mvhline(h - 1, 0, ' ', w);
        mvprintw(h - 1, 2, ":%s", cmd->command);
    } else if (msg[0] != '\0') {
        mvhline(h - 1, 0, ' ', w);
        mvprintw(h - 1, 2, "%s", msg);
    }
 
    move(1 + cur->y - scroll, 1 + cur->x);
    refresh();
}


// switches between modes like how vim does
Mode switch_mode(Mode mode, int ch) {
    if (mode == NORMAL && ch == 'i') return INSERT;
    if (mode == NORMAL && ch == ':') return COMMAND;
    if (mode == INSERT && ch == 27) return NORMAL;
    if (mode == COMMAND && ch == 27) return NORMAL;
    return mode;
}
