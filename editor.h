#ifndef EDITOR_H
#define EDITOR_H

#include <ncurses.h>

typedef enum {
	NORMAL, 
	INSERT, 
	COMMAND 
} Mode;

typedef struct {
    char **lines;
    int line_count;
    int capacity;
    int modified;
} Document;

typedef struct {
    int x, y;
} Cursor;

typedef struct {
    char command[128];
    int pos;
} CommandBuffer;

void init_document(Document *doc);
void free_document(Document *doc);
void insert_char(Document *doc, Cursor *cur, char ch);
void backspace_char(Document *doc, Cursor *cur);
void split_line(Document *doc, Cursor *cur);
void draw(Document *doc, Cursor *cur, Mode mode, int cmd_active, CommandBuffer *cmd, const char *msg, int scroll);
Mode switch_mode(Mode mode, int ch);
int save_file(Document *doc, const char *filename);
int load_file(Document *doc, const char *filename);

#endif
