#ifndef EDITOR_H
#define EDITOR_H
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <windows.h>
#include <share.h>
#include <Lmcons.h>
#define LIMIT_PATH_MAX MAX_PATH
#define LIMIT_HOST_NAME_MAX UNLEN+1
#define INIT_CONSOLE_DATA() \
    INPUT_RECORD irInBuf[128]; \
    uint64_t cNumRead, i;
#define FILE_WROPEN(fp, f) fp = _fsopen(f, "w", _SH_DENYRD);
#define FILE_AOPEN(fp, f) fp = _fsopen(f, "a+", _SH_DENYRD);
#define SLEEP(a) Sleep(a);
#define TERM_IO(n) HANDLE n
#define TERM DWORD
#define TERM_SET(m) SetConsoleMode(hStdin, m);
#elif defined(__linux__)
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <limits.h>
#define LIMIT_PATH_MAX PATH_MAX
#define LIMIT_HOST_NAME_MAX HOST_NAME_MAX
#define INIT_CONSOLE_DATA() \
   int nread; \
   char c;
#define FILE_WROPEN(fp, f) fp = fopen(f, "w");
#define FILE_AOPEN(fp, f) fp = fopen(f, "a+");
#define SLEEP(a) sleep(a);
#define TERM_IO(n) struct termios n
#define TERM struct termios
#define TERM_SET(m) tcsetattr(STDIN_FILENO, TCSAFLUSH, &m)
#endif
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include "lua5.4/lua.h"
#include "lua5.4/lauxlib.h"
#include "lua5.4/lualib.h"

#define MAX_LINE_LENGTH 80
#define LINE_SIZE 2048

typedef struct _Config {
    bool word_wrap;
} Config;

typedef struct _Buffer {
    char *lines[LINE_SIZE];
    uint64_t line_count;
    uint8_t cursor_pos;
    uint32_t current_line;
    char filename[LIMIT_PATH_MAX];
} Buffer;

typedef struct _UserInfo {
    char username[LIMIT_HOST_NAME_MAX];
    /* DWORD username_length; */
    uint64_t usernam_length;
#endif
    char config_path[LIMIT_PATH_MAX];
} UserInfo;

enum e_modifiers { M_CONTROL, M_ALT, M_SHIFT, MODIFIERS_COUNT };

enum e_configInitErrors { CONFIG_INIT_OK, CONFIG_INIT_CANNOT_READ_CONFIG };

typedef struct _Modifier {
    uint8_t keycode;
    bool isActive;
} Modifier;

/* typedef struct _Console_Info { */
/*     CONSOLE_SCREEN_BUFFER_INFO csbi; */
/*     HANDLE hStdin; */
/*     DWORD fdwSaveOldMode; */
/*     int columns, rows; */
/* } Console_Info; */

// TODO:
// implement this rt_variables (runtime variables)
// instead of having global variables
typedef struct _rt_variables {
    Config config;
    Buffer *buffers;
} rt_variables;


void drawUpdate(uint8_t bi);
char * getBufferLine(uint8_t bi, uint64_t line);
uint16_t getBufferLineCount(uint8_t bi);
void Exit(void);
/* VOID ErrorExit(LPSTR lpszMessage); */
/* void ErrorExit(long *message); */
void ErrorExit (char *messgae);
void initBuffer(uint8_t bi, char *buffer_file);
uint8_t KeyEventProc(uint8_t bi, KEY_EVENT_RECORD ker);
/* void MouseEventProc(MOUSE_EVENT_RECORD); */
uint8_t newBuffer(char *buffer_file);
void newlines(uint8_t bi, uint8_t count);
void notifyUpdate(uint8_t bi, uint8_t *status);
/* void ResizeEventProc(WINDOW_BUFFER_SIZE_RECORD); */
int runtime(int argc, char **argv);
uint8_t writeToFile(uint8_t i);
void writeToBuffer(uint8_t bi, uint64_t line, char *text, uint16_t len);
void initConsole();
HANDLE getSTdinHandle();
Modifier *getModifier(enum e_modifiers);
void initConfig(void);
void userInfoInit();
void setup();


#endif // EDITOR_H
