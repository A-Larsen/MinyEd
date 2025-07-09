#include "editor.h"
#include <stdint.h>

/* static HANDLE hStdin; */
/* static TERM_IO(hStdin); */
TERM_IO(hStdin);
/* static DWORD fdwSaveOldMode; */
static TERM fdwSaveOldMode;
static TERM current_term;
static int columns, rows;
static Buffer *buffers = NULL;
static Config config;
static UserInfo userInfo;

static Modifier modifiers[MODIFIERS_COUNT] = {
    [M_CONTROL] =  {.keycode = 17, .isActive = false} ,// control
    [M_ALT] =  {.keycode = 88, .isActive = false}, // alt
    [M_SHIFT] =  {.keycode = 16, .isActive = false} // shift
};

Modifier *getModifier(enum e_modifiers i) {
    return &modifiers[i];
}

void userInfoInit() {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    userInfo.username_length = UNLEN + 1;
    GetUserName(userInfo.username, &userInfo.username_length);
    sprintf(userInfo.config_path, "C:\\Users\\%s\\AppData\\Local\\MinyEd\\config.lua", userInfo.username);
#elif defined(__linux__)
    gethostname(userInfo.username, LIMIT_HOST_NAME_MAX);
    userInfo.username_length = strlen(userInfo.username);
    sprintf(userInfo.config_path, "C:\\Users\\%s\\AppData\\Local\\MinyEd\\config.lua", userInfo.username);
#endif
}

void setup() {
    // on initial lanch create default config
    userInfoInit();
    printf( "no config file found\n\n"
            "specify a path for your config file\n");
    /* printf("%s\n", userInfo.username); */

    /* char buffer[MAX_PATH] = {0}; */
    /* char path[MAX_PATH]; */
    sprintf(userInfo.config_path, "C:\\Users\\%s\\AppData\\Local\\MinyEd\\config.lua", userInfo.username);
    /* fgets(buffer, MAX_PATH, stdin); */
    printf("%s\n", userInfo.config_path);

}
void initConfig(void) {
    memset(&config, 0, sizeof(Config));
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    /* if(luaL_dofile(L, "config.lua") != LUA_OK) { */
    if(luaL_dofile(L, userInfo.config_path) != LUA_OK) {
        fprintf(stderr, "no config file\n");
        luaL_error(L, lua_tostring(L, -1));
        ErrorExit("cannot read config file");
    }

    lua_getglobal(L, "word_wrap");
    config.word_wrap = lua_tointeger(L, -1);
    lua_close(L);
}

void initConsole() {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
  
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD fdwMode;
    if (hStdin == INVALID_HANDLE_VALUE)
        ErrorExit("GetStdHandle");

    // Save the current input mode, to be restored on exit.

    if (! GetConsoleMode(hStdin, &fdwSaveOldMode) )
        ErrorExit("GetConsoleMode");

    // Enable the window and mouse input events.

    fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
    if (! SetConsoleMode(hStdin, fdwMode) )
        ErrorExit("SetConsoleMode");
#elif defined(__linux__)
    struct winsize ws;
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0){
        /* if (WRITE("\e999C" "\e999B", 12) != 12) */ 
        /*     return -1; */
        /* return term_getCursorPosition(TERM_IO); */
        ErrorExit("could not get terminal size");
    }
    rows = ws.ws_col;
    columns = ws.ws_row;

    if (tcgetattr(STDIN_FILENO, &fdwSaveOldMode)) {
        /* term_die("tcgetattr failed"); */
    }

    /* atexit(term_disableRawMode); */
    /* struct termios current_term; */

    current_term = fdwSaveOldMode;
    current_term.c_iflag &= !(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    current_term.c_oflag &= ~(OPOST);
    current_term.c_cflag |= (CS8);
    current_term.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    current_term.c_cc[VMIN] = 0;
    current_term.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &current_term)) {
        /* term_die("tcsetattr failed"); */
    }
#endif
}

TERM_IO() getSTdinHandle() {
    return hStdin;
}


void newlines(uint8_t bi, uint8_t count) {
    for (int i = buffers[bi].line_count; i < buffers[bi].line_count + count; ++i) {
        buffers[bi].lines[i] = malloc(sizeof(char) * LINE_SIZE);
        memset(buffers[bi].lines[i], 0, LINE_SIZE);
    }
    buffers[bi].line_count += count;
}

uint8_t writeToFile(uint8_t bi) {
    // TODO:
    // remove existing file before writing so that duplicate text isn't created.
    /* FILE *fp = _fsopen(buffers[bi].filename, "w", _SH_DENYRD); */
    /* FILE *fp = fopen(buffers[bi].filename, "w"); */
    FILE *fp = NULL;
    FILE_WROPEN(fp, buffers[bi].filename);
    for (uint64_t i = 0; i < buffers[0].line_count; ++i) {
        int len = strlen(buffers[0].lines[i]);
        fwrite(buffers[0].lines[i], len, 1, fp);
    }
    fclose(fp);

    // return a  status to display after a file write
    return 1;
}

void initBuffer(uint8_t bi, char *buffer_file) {
    Buffer *buffer = &buffers[bi];
    memcpy(buffer->filename, buffer_file, strlen(buffer_file));
    /* FILE *fp = _fsopen(buffers[bi].filename, "a+", _SH_DENYRD); */
    FILE *fp = NULL;
    FILE_AOPEN(fp, buffers[bi].filename);

    if (fp == NULL) return;
        /* ErrorExit("cannot open file"); */

    char line[LINE_SIZE];
    uint64_t i = 0;
    while(fgets(line, LINE_SIZE, fp) != NULL) {
        uint16_t len = strlen(line);
        memcpy(buffers[0].lines[i], line, len);
        newlines(bi, 1);
        buffers[bi].cursor_pos = len;
        i++;
    }
    fclose(fp);
}

uint8_t newBuffer(char *buffer_file) {
    static uint8_t count = 0;
    buffers = (Buffer *)malloc(sizeof(Buffer));
    buffers[count].line_count = 0;
    buffers[count].current_line = 0;
    if (buffers == NULL) {
        ErrorExit("cannot allocate a new buffer");
    }
    newlines(count, 1);
    initBuffer(count, buffer_file);
    count++;
    return count - 1;
}

void writeToBuffer(uint8_t bi, uint64_t line, char *text, uint16_t len) {
    // TODO:
    // for each newline in string make new line
    memcpy(buffers[bi].lines[line], text, len);
}

char * getBufferLine(uint8_t bi, uint64_t line) {
    return buffers[bi].lines[line];
}

char ** getBufferLines(uint8_t bi) {
    return buffers[bi].lines;
}

uint16_t getBufferLineCount(uint8_t bi) {
    return buffers[bi].line_count;
}

void Exit(void)
{
    printf("\e[1;1H\e[2J"); // clear screen
    /* SetConsoleMode(hStdin, fdwSaveOldMode); */
    TERM_SET(fdwSaveOldMode);
    exit(0);
}

/* VOID ErrorExit (LPSTR lpszMessage) */
void ErrorExit (char *message)
{
    printf("\e[1;1H\e[2J"); // clear screen
    fprintf(stderr, "%s\n", message);
    Exit();
}

void notifyUpdate(uint8_t bi, uint8_t *status) {
    if (*status == 0) return;

    printf("\e[42m"); // background green
    printf("\e[30m"); // foreground black

    switch(*status) {
        case 1: {
            char buffer[225];
            sprintf(buffer, "wrote to file: \"\e[34m%s\e[30m\"", buffers[bi].filename);
            uint16_t len = strlen(buffer);
            printf("\e[%d;%dH", 1 , columns - len - 1);
            printf("%s", buffer);
            break;
        }
    }
    /* Sleep(1000); */
    SLEEP(1000);
    /* *status = 0; */
    printf("\e[0m"); // default colors

}

void drawUpdate(uint8_t bi) {
    // Eventually I will get rid of these comments and just make macros for
    // theses calls to escape sequences. So it'll clean things up
    printf("\e[1;1H\e[2J"); // clear screen
    Buffer *buffer = &buffers[bi];
                            //
    for (int i = 0; i < buffer->line_count; ++i) {
        printf("%s", buffer->lines[i]);
    }
    printf("\e[42m"); // background green
    printf("\e[30m"); // foreground black
                      //

    printf("\e[%d;%dH", rows , 5);
    printf("%s", buffer->filename);

    printf("\e[%d;%dH", rows , columns - 5);
    printf("%d, %d", buffer->current_line + 1, buffer->cursor_pos + 1);
                     //
    printf("\e[%d;%dH", buffer->current_line + 1, buffer->cursor_pos + 1);
    printf("\e[0m"); // default colors
}

void wrapLine(uint8_t bi, int line_len) {
    Buffer *buffer = &buffers[bi];
    while (buffer->lines[buffer->current_line][line_len] != ' ') line_len--;
        char *word = buffers->lines[buffer->current_line] + line_len + 1;
        newlines(bi, 1);
        buffer->current_line++;

        char *p = buffer->lines[buffer->current_line];
        int word_len = strlen(word);
        memcpy(p, word, word_len);
        buffer->lines[buffer->current_line - 1][line_len] = '\n';
        char *p2 = buffers[0].lines[buffer->current_line - 1] + line_len + 1;
        memset(p2, 0, strlen(p2));
        buffer->cursor_pos = word_len;
}

// wraps lines from the current line and below. End when there is not a line
// to wrap or if there is nothing in the line
void wrapLines(uint8_t bi) {

    Buffer *buffer = &buffers[bi];
    printf("\n%lu\n", buffer->line_count);
    /* Sleep(1000); */
    /* return; */

    char *word = NULL;
    int word_len = 0;

    // for now this is just one itteration though all of the lines but it might
    // need more than one through all the lines if there are multiple words
    // longer than the line length
    for (int i = 0; i < buffer->line_count; ++i) {
        int line_len = strlen(buffer->lines[i]);
        /* printf("\n%d: %d\n", buffer->line_count, i); */

        if (word_len > 0) {
            for (int j = line_len; j > word_len; --j) {
                buffer->lines[i][j + word_len] = buffer->lines[i][j];
            }
            memcpy(buffer->lines[i], word, word_len);
            line_len = strlen(buffer->lines[i]);
        }

        word = NULL;
        word_len = 0;

        if (line_len > MAX_LINE_LENGTH) {
            uint16_t bt = line_len;
            while (buffer->lines[i][bt] != ' ') bt--;
            bt += 1;
            char *word = buffers->lines[i] + bt;
            word_len = strlen(word);
        }

        
    }
}

// @ker
//     A key even record from an event stream
// return
//     Whatever is returned in this function detirms the next action that should
//     be taken.
uint8_t KeyEventProc(uint8_t bi, KEY_EVENT_RECORD ker)
{
    Buffer *buffer = &buffers[bi];
    // up -> 38
    // down -> 40
    // right -> 39
    // left -> 37
    static int word_wrap_pos;
    static int previous_ch = 0;

    // it might be more benifitial to eventually access the line length from anywhere
    int line_len = strlen(buffer->lines[buffer->current_line]);

    int ch = ker.uChar.AsciiChar;
    if (!ker.bKeyDown) return 0;
    if (ker.wVirtualKeyCode == M_SHIFT) return 0;

    /* if (modifiers[M_CONTROL].isActive && ker.wVirtualKeyCode == 0x56 ) // control-v */
    /*     wrapLines(bi); */


    if (buffer->cursor_pos >= MAX_LINE_LENGTH && config.word_wrap) { // word wrap
        wrapLine(bi, line_len);
    } 

    if (ker.wVirtualKeyCode == 38) {
        buffer->current_line--;
    } 
    if (ker.wVirtualKeyCode == 13) { // enter
        buffer->lines[buffer->current_line][buffer->cursor_pos] = '\n';
        newlines(bi, 1);
        buffer->current_line++;
        buffer->cursor_pos = 0;
    } else if (ker.wVirtualKeyCode == 8) { //backspace
        // TODO:
        // change when cursor position is not at the end of the line
        int len = strlen(buffer->lines[buffer->current_line]);
    buffer->lines[buffer->current_line][len - 1] = '\0';
        buffer->cursor_pos--;
    } else if (modifiers[M_CONTROL].isActive && ker.wVirtualKeyCode == 87 ) { // control-w
        return 1;
    } else if (modifiers[M_CONTROL].isActive && ker.wVirtualKeyCode == 0x43 ) { // control-c
        Exit();
    } else if (isalpha(ch) || isspace(ch) || ispunct(ch)){ // print character
        /* char *line = lines[current_line]; */
        if (buffer->cursor_pos < line_len - 1) {
            // In this case shift everything to the right of the cursor pos. The
            // action might shift over to multiple lines if word wrap is on
            for (int i = LINE_SIZE - 1; i > buffer->cursor_pos; --i) {
                buffer->lines[buffer->current_line][i] = buffer->lines[buffer->current_line][i - 1];
                line_len = strlen(buffer->lines[buffer->current_line]);
                if (config.word_wrap) {
                    if (line_len >= MAX_LINE_LENGTH) {

                    }
                    // shift all the necessary lines over


                }
            }

            
        }
        buffer->lines[buffer->current_line][buffer->cursor_pos] = ch;
        previous_ch = ch;
        buffer->cursor_pos++;
    } 

    return 0;
}

/* void MouseEventProc(MOUSE_EVENT_RECORD mer) */
/* { */
/* #ifndef MOUSE_HWHEELED */
/* #define MOUSE_HWHEELED 0x0008 */
/* #endif */
/*     printf("Mouse event: "); */

/*     switch(mer.dwEventFlags) */
/*     { */
/*         case 0: */

/*             if(mer.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED) */
/*             { */
/*                 printf("left button press \n"); */
/*             } */
/*             else if(mer.dwButtonState == RIGHTMOST_BUTTON_PRESSED) */
/*             { */
/*                 printf("right button press \n"); */
/*             } */
/*             else */
/*             { */
/*                 printf("button press\n"); */
/*             } */
/*             break; */
/*         case DOUBLE_CLICK: */
/*             printf("double click\n"); */
/*             break; */
/*         case MOUSE_HWHEELED: */
/*             printf("horizontal mouse wheel\n"); */
/*             break; */
/*         case MOUSE_MOVED: */
/*             printf("mouse moved\n"); */
/*             break; */
/*         case MOUSE_WHEELED: */
/*             printf("vertical mouse wheel\n"); */
/*             break; */
/*         default: */
/*             printf("unknown\n"); */
/*             break; */
/*     } */
/* } */

/* void ResizeEventProc(WINDOW_BUFFER_SIZE_RECORD wbsr) */
/* { */
/*     printf("Resize event\n"); */
/*     printf("Console screen buffer is %d columns by %d rows.\n", wbsr.dwSize.X, wbsr.dwSize.Y); */
/* } */
