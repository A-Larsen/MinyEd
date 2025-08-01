// TODO
// - Fix what happens when writing at the end of a line before a previous one and
//   the new line escape sequence being removed
//
// - Fix line wrapping
//
// - Fix extra spaces in buffer after saving a file
//
// - only redraw what the areas that need redrawing. not everything
//
// - change buffers feature
//
// - highlight text feature
//
// - modal option (like vim). This would work with the config file. instead of
//   keys being in switch for the key processing it should be variables that can
//   be changed to any key
//
// - stop reload buffer from missing one character when there is no new line
#include "editor.h"
#include <stdint.h>

static HANDLE hStdin;
static DWORD fdwSaveOldMode;
static int columns, rows;
static Buffer *buffers = NULL;
static Config config;
static UserInfo userInfo;
static HDC screen_context;
static uint8_t status_id = STAT_NONE;

static Modifier modifiers[MODIFIERS_COUNT] = {
    [M_CONTROL] =  {.keycode = 17, .isActive = false} ,// control
    [M_ALT] =  {.keycode = 88, .isActive = false}, // alt
    [M_SHIFT] =  {.keycode = 16, .isActive = false} // shift
};

Modifier *getModifier(enum e_modifiers i) {
    return &modifiers[i];
}

void userInfoInit() {
    userInfo.username_length = UNLEN + 1;
    GetUserName(userInfo.username, &userInfo.username_length);
    sprintf(userInfo.config_path, "C:\\Users\\%s\\AppData\\Local\\MinyEd\\config.lua", userInfo.username);
}

void setup() {
    // on initial lanch create default config
    userInfoInit();
    printf( "no config file found\n\n"
            "specify a path for your config file\n");
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
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    screen_context = GetDC(NULL);
    HFONT hFont1 = CreateFont(48,0,0,0,FW_DONTCARE,FALSE,TRUE,FALSE,DEFAULT_CHARSET,OUT_OUTLINE_PRECIS,
        CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY, VARIABLE_PITCH,TEXT("Impact"));

  
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
}

HANDLE getSTdinHandle() {
    return hStdin;
}

void newlines(uint8_t bi, uint8_t count) {
    for (int i = buffers[bi].line_count; i < buffers[bi].line_count + count; ++i) {
        buffers[bi].lines[i] = malloc(sizeof(char) * LINE_SIZE);
        memset(buffers[bi].lines[i], 0, LINE_SIZE);
    }
    buffers[bi].line_count += count;
}

void writeToFile(uint8_t bi) {
    Buffer *buffer = &buffers[bi];
    // TODO:
    // remove existing file before writing so that duplicate text isn't created.
    FILE *fp = _fsopen(buffer->filename, "w", _SH_DENYRD);
    for (uint64_t i = 0; i < buffer->line_count; ++i) {
        int len = strlen(buffer->lines[i]);
        fwrite(buffer->lines[i], len, 1, fp);
        fwrite("\n", 1, 1, fp);
    }
    fclose(fp);
    status_id = STAT_FILE_WRITE;
    // return a  status to display after a file write
}

void initBuffer(uint8_t bi, char buffer_file[MAX_PATH]) {
    // clear buffer and then write to it
    Buffer *buffer = &buffers[bi];
    memcpy(buffer->filename, buffer_file, strlen(buffer_file));
    FILE *fp = _fsopen(buffer->filename, "a+", _SH_DENYRD);

    if (fp == NULL) return;
        /* ErrorExit("cannot open file"); */

    char line[LINE_SIZE];
    uint64_t i = 0;
    while(fgets(line, LINE_SIZE, fp) != NULL) {
        uint16_t len = strlen(line);
        uint16_t lencpy = len;
        if (strstr(line, "\n")) lencpy--;
        memcpy(buffer->lines[i], line, lencpy);
        newlines(bi, 1);
        buffer->cursor_pos = len;
        i++;
    }
    fclose(fp);
}

uint8_t newBuffer(char buffer_file[MAX_PATH]) {
    static uint8_t count = 0;
    if (buffers == NULL) {
        buffers = (Buffer *)malloc(sizeof(Buffer));
    } else {
        buffers = (Buffer *)realloc(buffers, sizeof(Buffer) * (count + 1));
    }
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

void reloadBuffer(uint8_t bi)  {
    printf("\e[1;1H\e[2J"); // clear screen
    char filename[MAX_PATH];
    memset(filename, 0, MAX_PATH);
    memcpy(filename, buffers[bi].filename, MAX_PATH);
    for (int i = 0; i < buffers[bi].line_count; ++i) {
        free((void *)buffers[bi].lines[i]);
        buffers[bi].lines[i] = NULL;
    }
    buffers[bi].line_count = 0;
    buffers[bi].current_line = 0;
    newlines(bi, 1);
    initBuffer(bi, filename);
    status_id = STAT_BUFFER_RELOAD;
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

VOID Exit(void)
{
    printf("\e[1;1H\e[2J"); // clear screen
    SetConsoleMode(hStdin, fdwSaveOldMode);
    ExitProcess(0);
}

VOID ErrorExit (LPSTR lpszMessage)
{
    printf("\e[1;1H\e[2J"); // clear screen
    fprintf(stderr, "%s\n", lpszMessage);
    Exit();
}

void notifyUpdate(uint8_t bi) {

    if (status_id == STAT_NONE) return;
    printf("\e[42m"); // background green
    printf("\e[30m"); // foreground black

    switch (status_id) {
        case STAT_FILE_WRITE: {
            char buffer[225];
            sprintf(buffer, "wrote to file: \"\e[34m%s\e[30m\"", buffers[bi].filename);
            uint16_t len = strlen(buffer);
            printf("\e[%d;%dH", 1 , columns - len - 1);
            printf("%s", buffer);
            break;
        }

        case STAT_BUFFER_RELOAD: {
            char buffer[225];
            sprintf(buffer, "reloaded buffer: \"\e[34m%s\e[30m\"", buffers[bi].filename);
            uint16_t len = strlen(buffer);
            printf("\e[%d;%dH", 1 , columns - len - 1);
            printf("%s", buffer);
            break;
        }
    }

    Sleep(1000);
    printf("\e[0m"); // default colors
    status_id = STAT_NONE;
}

void drawUpdate(uint8_t bi) {
    // Eventually I will get rid of these comments and just make macros for
    // theses calls to escape sequences. So it'll clean things up
    printf("\e[1;1H\e[2J"); // clear screen
    Buffer *buffer = &buffers[bi];
                            //
    for (int i = 0; i < buffer->line_count; ++i) {
        printf("%s\n", buffer->lines[i]);
    }
    printf("\e[42m"); // background green
    printf("\e[30m"); // foreground black
                      //

    printf("\e[%d;%dH", rows , 5);
    printf("%s", buffer->filename);

    printf("\e[%d;%dH", rows , columns - 5);
    printf("%d, %d", buffer->current_line + 1, buffer->cursor_pos + 1);
    printf("\e[%d;%dH", buffer->current_line + 1, buffer->cursor_pos + 1);
    printf("\e[0m"); // default colors
}

void wrapLine(uint8_t bi, int line_len) {
    // TODO
    // have word wrap remove trailing spaces at the end of a line
    Buffer *buffer = &buffers[bi];
    while (buffer->lines[buffer->current_line][line_len] != ' ') line_len--;
        char *word = buffers->lines[buffer->current_line] + line_len + 1;
        newlines(bi, 1);
        buffer->current_line++;

        char *p = buffer->lines[buffer->current_line];
        int word_len = strlen(word);
        memcpy(p, word, word_len);
        buffer->lines[buffer->current_line - 1][line_len] = '\0';
        char *p2 = buffers[0].lines[buffer->current_line - 1] + line_len + 1;
        memset(p2, 0, strlen(p2));
        buffer->cursor_pos = word_len;
}

// wraps lines from the current line and below. End when there is not a line
// to wrap or if there is nothing in the line
void wrapLines(uint8_t bi) {

    Buffer *buffer = &buffers[bi];
    printf("\n%llu\n", buffer->line_count);
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
            /* char *word = buffers->lines[i] + bt; */
            char *word = buffer->lines[i] + bt;
            word_len = strlen(word);
        }

        
    }
}

void writePrintableCharacters(uint8_t bi, int ch, int *previous_ch, int line_len) {

    Buffer *buffer = &buffers[bi];
     if (isalpha(ch) || isspace(ch) || ispunct(ch)) {
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
        *previous_ch = ch;
            
        buffer->cursor_pos++;

     }
}

void moveVertical(uint8_t bi, int *line_len, bool upOrDown) {
    Buffer *buffer = &buffers[bi];
    if (upOrDown) {
        if (buffer->current_line + 1 >= buffer->line_count) return;
        buffer->current_line++;
    } else {
        if (buffer->current_line <= 0) return;
        buffer->current_line--;
    }
    
    *line_len = strlen(buffer->lines[buffer->current_line]);
    if (buffer->cursor_pos >= *line_len) buffer->cursor_pos = *line_len;
}

// @ker
//     A key even record from an event stream
// return
//     Whatever is returned in this function detirms the next action that should
//     be taken.
void KeyEventProc(uint8_t bi, KEY_EVENT_RECORD ker)
{
    Buffer *buffer = &buffers[bi];
    static int word_wrap_pos;
    static int previous_ch = 0;

    // it might be more benifitial to eventually access the line length from anywhere
    int line_len = strlen(buffer->lines[buffer->current_line]);

    int ch = ker.uChar.AsciiChar;
    if (!ker.bKeyDown) return;
    if (ker.wVirtualKeyCode == M_SHIFT) return;


    if (buffer->cursor_pos >= MAX_LINE_LENGTH && config.word_wrap) { // word wrap
        wrapLine(bi, line_len);
    } 

    // TODO 
    // instead of having perminate keys for each action have a mutal 'key'
    // for each action.
    // handle virtual keys
    switch (ker.wVirtualKeyCode) {

        case VK_RETURN: {
            /* buffer->lines[buffer->current_line][buffer->cursor_pos] = '\n'; */
            newlines(bi, 1);
            buffer->current_line++;
            buffer->cursor_pos = 0;
            break;
        }
        // TODO
        // When doing cursor moving remember to do some bounds checking
        case VK_LEFT: {
            if (buffer->cursor_pos > 0) buffer->cursor_pos--;
            break;
        }

        case VK_RIGHT: {
            // line_len - 1 to adjust for the newline symbol
            if (buffer->cursor_pos < line_len) buffer->cursor_pos++;
            break;
        }

        case VK_UP: {moveVertical(bi, &line_len, false); break; }

        case VK_DOWN: {moveVertical(bi, &line_len, true); break;}


        case VK_BACK: {
            // TODO:
            // wrap the next line to the line before it if need be
            /* if ((buffer->cursor_pos - 1) < 0) { */
            /*     buffer->current_line--; */
            /*     buffer->cursor_pos = strlen(buffer->lines[buffer->current_line]); */
            /* } */
            if (buffer->cursor_pos - 1 < 0) break;
            int len = strlen(buffer->lines[buffer->current_line]);
            for (int i = (--buffer->cursor_pos); i < len; ++i) {
                buffer->lines[buffer->current_line][i] = buffer->lines[buffer->current_line][i + 1];
            }

            break;
        }

        case 'N' : { // next buffer
            if (modifiers[M_CONTROL].isActive) {
                if (current_buffer_id + 1 < buffer_count) {
                    current_buffer_id++; 
                } else {
                    current_buffer_id = 0;
                }
                break;
            }
        }

        case 'P' : { // previous buffer
            if (modifiers[M_CONTROL].isActive) {
                if (current_buffer_id - 1 >= 0 ) {
                    current_buffer_id--; 
                } else {
                    current_buffer_id = buffer_count - 1;
                }
                break;
            }
        }

        case 'R' : { // reload buffer
            if (modifiers[M_CONTROL].isActive) {reloadBuffer(bi); break;}
        }

        case 'W': {
            if (modifiers[M_CONTROL].isActive) {writeToFile(bi); break;}
        }

        case 'C': {
            if (modifiers[M_CONTROL].isActive) Exit();
        }

        default: writePrintableCharacters(bi, ch, &previous_ch, line_len); break;

    }
}

VOID MouseEventProc(MOUSE_EVENT_RECORD mer)
{
#ifndef MOUSE_HWHEELED
#define MOUSE_HWHEELED 0x0008
#endif
    printf("Mouse event: ");

    switch(mer.dwEventFlags)
    {
        case 0:

            if(mer.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
            {
                printf("left button press \n");
            }
            else if(mer.dwButtonState == RIGHTMOST_BUTTON_PRESSED)
            {
                printf("right button press \n");
            }
            else
            {
                printf("button press\n");
            }
            break;
        case DOUBLE_CLICK:
            printf("double click\n");
            break;
        case MOUSE_HWHEELED:
            printf("horizontal mouse wheel\n");
            break;
        case MOUSE_MOVED:
            printf("mouse moved\n");
            break;
        case MOUSE_WHEELED:
            printf("vertical mouse wheel\n");
            break;
        default:
            printf("unknown\n");
            break;
    }
}

VOID ResizeEventProc(WINDOW_BUFFER_SIZE_RECORD wbsr)
{
    printf("Resize event\n");
    printf("Console screen buffer is %d columns by %d rows.\n", wbsr.dwSize.X, wbsr.dwSize.Y);
}
