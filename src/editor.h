#ifndef EDITOR_H
#define EDITOR_H
#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <share.h>
#include <string.h>
#include <Lmcons.h>
#include "lua5.4/lua.h"
#include "lua5.4/lauxlib.h"
#include "lua5.4/lualib.h"
#include "wingdi.h"

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
    char filename[MAX_PATH];
} Buffer;

typedef struct _UserInfo {
    char username[UNLEN+1];
    DWORD username_length;
    char config_path[MAX_PATH];
} UserInfo;

enum e_modifiers { M_CONTROL, M_ALT, M_SHIFT, MODIFIERS_COUNT };

enum e_configInitErrors { CONFIG_INIT_OK, CONFIG_INIT_CANNOT_READ_CONFIG };

enum e_status {STAT_FILE_WRITE = 1, STAT_BUFFER_RELOAD};

typedef struct _Modifier {
    uint8_t keycode;
    bool isActive;
} Modifier;

typedef struct _Console_Info {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hStdin;
    DWORD fdwSaveOldMode;
    int columns, rows;
} Console_Info;

// TODO:
// implement this rt_variables (runtime variables)
// instead of having global variables
typedef struct _rt_variables {
    Config config;
    Buffer *buffers;
} rt_variables;

extern uint8_t current_buffer_id;
extern uint8_t buffer_count;

void drawUpdate(uint8_t bi);
char * getBufferLine(uint8_t bi, uint64_t line);
uint16_t getBufferLineCount(uint8_t bi);
VOID Exit(void);
VOID ErrorExit(LPSTR lpszMessage);
void initBuffer(uint8_t bi, char *buffer_file);
void KeyEventProc(uint8_t bi, KEY_EVENT_RECORD ker);
VOID MouseEventProc(MOUSE_EVENT_RECORD);
uint8_t newBuffer(char *buffer_file);
void newlines(uint8_t bi, uint8_t count);
void notifyUpdate(uint8_t bi);
VOID ResizeEventProc(WINDOW_BUFFER_SIZE_RECORD);
void writeToFile(uint8_t i);
void writeToBuffer(uint8_t bi, uint64_t line, char *text, uint16_t len);
void initConsole();
HANDLE getSTdinHandle();
Modifier *getModifier(enum e_modifiers);
void initConfig(void);
void userInfoInit();
void setup();


#endif // EDITOR_H
