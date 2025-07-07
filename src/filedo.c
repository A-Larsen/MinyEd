#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <share.h>
#include <malloc.h>
#include "filedo_options.h"


int main(int argc, char **argv)
{
    // command line arguments
    // 1. filename
    // 2. what to do
    //    1.1 read
    //    1.2 findword

    HANDLE HFile = CreateFileA(argv[1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL, NULL);

    if (HFile == INVALID_HANDLE_VALUE) {
        printf("file does not exist\n");
        /* fclose(H); */
        CloseHandle(HFile);
        return 1;
    }

    if (strcmp(argv[2], "read\0") == 0) {
        /* printf("reading\n"); */
        readfile(HFile);
    } else if (strcmp(argv[2], "findword") == 0) {
        findword(HFile);
    }

    CloseHandle(HFile);

    return 0;
}
