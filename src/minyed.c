#include "editor.h"

int main(int argc, char **argv)
{     

    initConsole();
    userInfoInit();
    /* DWORD cNumRead, i; */

    uint64_t cNumRead, i;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    INPUT_RECORD irInBuf[128];
    HANDLE hStdin = getSTdinHandle();
#elif defined(__linux__)
    uint8_t hStdin = STDIN_FILENO;
    char c;
#endif
    /* INIT_CONSOLE_DATA(); */

    printf("\e[1;1H\e[2J"); // clear screen
                            //
    // on initial lanch create default config
    /* switch(1) { */
    /*     case CONFIG_INIT_CANNOT_READ_CONFIG: {setup(); break;} */
    /* } */
    /* return 0; */

    initConfig();


    if (argc < 2) {
        fprintf(stderr, "not enough arguments\n");
        return 1;
    }

    newBuffer(argv[1]);

    uint8_t dowhat = 0;
    bool first_input = false;
    /* HANDLE hStdin = getSTdinHandle(); */
    /* TERM_IO(hStdin) = getSTdinHandle(); */

    while (true)
    {
        uint8_t status = 0;
        // Wait for the events.

        /* if (! ReadConsoleInput( */
        /*         hStdin,      // input buffer handle */
        /*         irInBuf,     // buffer to read into */
        /*         128,         // size of read buffer */
        /*         &cNumRead) */ 
        /*         ) // number of records read */
        /* if (! ReadConsoleInput(hStdin, irInBuf, 128, &cNumRead)) // number of records read */
        /* if (! TERM_READ(hStdin, &c, 1, cNumRead)) // number of records read */
        /*     ErrorExit("ReadConsoleInput"); */

         TERM_READ(hStdin, &c, 1, cNumRead);
        // Dispatch the events to the appropriate handler.
        // a dowhat activity can return a id for a status to display
        notifyUpdate(0, &status);

        for (i = 0; i < cNumRead; i++)
        {
            /* switch(irInBuf[i].EventType) { case KEY_EVENT:{  KeyEventProc(0, irInBuf[i].Event.KeyEvent.uChar.AsciiChar); break; } } */
            TERM_PROCESS(KeyEventProc(0, c))
        }
        drawUpdate(0);
    }

    Exit();

    return 0;

}
