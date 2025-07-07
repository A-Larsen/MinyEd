#include "filedo_options.h"
#include <stdint.h>

long findword(HANDLE HFile)
{
    /* const int linesize = 81; */
    /* long unsigned int linenumber = 1; */
    /* char line[linesize]; // a line cannot be more than 80 characters. Including one */
                         // extra character for the null terminator
    /* while (fgets(line, sizeof(line), fp) != NULL){ */
    /*     printf("%lu: %s\n", linenumber, line); */
    /*     linenumber++; */
    /* } */

    return 0;
}

void
readfile(HANDLE HFile)
{
    DWORD size = GetFileSize(HFile, NULL);
    char *buffer = (char *)malloc((size_t)size + 1);
    memset(buffer, 0, size + 1);
    printf("size: %lu\n", size);
    ReadFile(HFile, buffer, size, 0, NULL);
    printf("file:\n%s", buffer);
    free(buffer);

}
