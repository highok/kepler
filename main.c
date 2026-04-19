#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include "editor.h"

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 0;
    }

    const char *filename = argv[1];

    initscr();			// initialize the screen with ncurses.
    raw();
    keypad(stdscr, TRUE);	// able to use keypads like KEY_UP, KEY_DOWN, F1, F2, ...
    noecho();			// don't echo when getch() is executed
    // curs_set(1);
    move(0, 0);

    Document doc;
    init_document(&doc);

    char msg[256] = {0};
    int msg_timer = 0;		// timing for the message

    if (load_file(&doc, filename) == 0) {
        snprintf(msg, sizeof(msg), "\"%s\" %d lines", filename, doc.line_count);
        msg_timer = 1;
    } else {
        snprintf(msg, sizeof(msg), "\"%s\" [new file]", filename);
        msg_timer = 1;
    }

    Cursor cur = {0, 0};	// initially the cursor will be at the beginning of the screen
    Mode mode = NORMAL;
    CommandBuffer cmd = {0};
    int cmd_active = 0;
    int scroll = 0;

    int quit = 0;
    while (!quit) {
	int height, weight;
	getmaxyx(stdscr, height, weight);
	int visible = height - 3;

	if (cur.y < scroll)
            scroll = cur.y;
        if (cur.y >= scroll + visible)
            scroll = cur.y - visible + 1;

        draw(&doc, &cur, mode, cmd_active, &cmd, msg, scroll);

        if (msg_timer > 0) msg_timer--;
        else msg[0] = '\0';

        int ch = getch();
        Mode old_mode = mode;
        mode = switch_mode(mode, ch);

	/* INSERT mode.
	 You can move the cursor using arrow keys.
	*/
        if (mode == INSERT) {
            if (ch == KEY_LEFT && cur.x > 0) cur.x--;
            if (ch == KEY_RIGHT) cur.x++;
            if (ch == KEY_DOWN && cur.y < doc.line_count - 1) cur.y++;
            if (ch == KEY_UP && cur.y > 0) cur.y--;

            if (old_mode == NORMAL) continue;

            if (ch == '\n') {
                split_line(&doc, &cur);
	    // BACKSPACE and DELETE will remove the preceding character
            } else if (ch == KEY_BACKSPACE || ch == 127) {
                backspace_char(&doc, &cur);
	    // all the characters including the specials will be inserted except the DELETE key.
            } else if (ch >= 32 && ch < 127) {
                insert_char(&doc, &cur, ch);
            }
        }
	// In vim we use hjkl to navigate through the file. But still the default keys works.
        else if (mode == NORMAL) {
            if ((ch == 'h' || ch == KEY_LEFT) && cur.x > 0) cur.x--;
            if (ch == 'l' || ch == KEY_RIGHT) cur.x++;
            if (ch == 'j' || (ch == KEY_DOWN && cur.y < doc.line_count - 1)) cur.y++;
            if ((ch == 'k' || ch == KEY_UP) && cur.y > 0) cur.y--;
        }
	/* Command block
	 * Here you can enter commands to perform operations on the currently opened file.
	 */
        else if (mode == COMMAND) {
            cmd_active = 1;
            if (old_mode == NORMAL && ch == ':') continue;

            if (ch == '\n') {
		// quit the editor
                if (strcmp(cmd.command, "q") == 0) {
                    if (doc.modified) {
                        snprintf(msg, sizeof(msg), "Changes has been done, but not saved. Use :wq or :w");
                        msg_timer = 2;
                        mode = NORMAL;
                        cmd_active = 0;
                        memset(&cmd, 0, sizeof(cmd));
                    } else {
                        quit = 1;
                    }
                }
		// save/write changes to the file
                else if (strcmp(cmd.command, "w") == 0) {
                    if (save_file(&doc, filename) == 0) {
                        snprintf(msg, sizeof(msg), "\"%s\" written", filename);
                        msg_timer = 2;
                    } else {
                        snprintf(msg, sizeof(msg), "Error: cannot write \"%s\"", filename);
                        msg_timer = 2;
                    }
                    mode = NORMAL;
                    cmd_active = 0;
                    memset(&cmd, 0, sizeof(cmd));
                }
		// save and quit
                else if (strcmp(cmd.command, "wq") == 0 || strcmp(cmd.command, "x") == 0) {
                    if (save_file(&doc, filename) == 0) {
                        quit = 1;
                    } else {
                        snprintf(msg, sizeof(msg), "Error: cannot write \"%s\"", filename);
                        msg_timer = 3;
                        mode = NORMAL;
                        cmd_active = 0;
                        memset(&cmd, 0, sizeof(cmd));
                    }
                }
                else if (cmd.command[0] != '\0') {
                    snprintf(msg, sizeof(msg), "Not a command: %s", cmd.command);
                    msg_timer = 2;
                    mode = NORMAL;
                    cmd_active = 0;
                    memset(&cmd, 0, sizeof(cmd));
                }
		// by default editor is in NORMAL mode
                else {
                    mode = NORMAL;
                    cmd_active = 0;
                    memset(&cmd, 0, sizeof(cmd));
                }
            }
            else if (ch == 27) {
                memset(&cmd, 0, sizeof(cmd));
                mode = NORMAL;
                cmd_active = 0;
            }
            else if (ch == KEY_BACKSPACE || ch == 127) {
                if (cmd.pos > 0)
                    cmd.command[--cmd.pos] = '\0';
            }
            else if (ch >= 32 && ch < 127) {
                if (cmd.pos < 127) {
                    cmd.command[cmd.pos++] = ch;
                    cmd.command[cmd.pos] = '\0';
                }
            }
        }

        if (cur.x > (int)strlen(doc.lines[cur.y]))
            cur.x = strlen(doc.lines[cur.y]);
    }

    free_document(&doc);
    endwin();
    return 0;
}
