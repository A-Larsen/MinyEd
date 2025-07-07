#ifndef _H
#define _H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <share.h>
#include <malloc.h>
#include <windows.h>
#include <fileapi.h>

/* #pragma comment(lib,"Kernal32.lib") */

enum E_OPTIONS {
    OPTION_READ = 1,
    OPTION_FIND_WORD,
    OPTIONS_SIZE
};


// DESCRIPTION:
//     Print the line that a specified word was found on with the word in a
//     highlighted color
//
// RETURNS:
//     Location of the found word.
//
long findword(HANDLE HFile);

// DESCRIPTION:
//    Prints text of the specified file 
//
void
readfile(HANDLE HFile);

#endif
