#include "editor.h"


int main(int argc, char **argv)
{     

    initConsole();
    DWORD cNumRead, i;
    INPUT_RECORD irInBuf[128];

    printf("\e[1;1H\e[2J"); // clear screen
                            //

    initConfig();



    if (argc < 2) {
        fprintf(stderr, "not enough arguments\n");
        return 1;
    }

    newBuffer(argv[1]);

    uint8_t dowhat = 0;
    bool first_input = false;
    HANDLE hStdin = getSTdinHandle();

    while (true)
    {
        uint8_t status = 0;
        // Wait for the events.

        if (! ReadConsoleInput(
                hStdin,      // input buffer handle
                irInBuf,     // buffer to read into
                128,         // size of read buffer
                &cNumRead) ) // number of records read
            ErrorExit("ReadConsoleInput");

        // Dispatch the events to the appropriate handler.
        // a dowhat activity can return a id for a status to display
        switch(dowhat) {
            case 1: {status = writeToFile(0); break;};
        }
        notifyUpdate(0, &status);

        for (i = 0; i < cNumRead; i++)
        {
            switch(irInBuf[i].EventType)
            {
                case KEY_EVENT:{ // keyboard input
                        for (int i = 0; i < MODIFIERS_COUNT; ++i) {
                            Modifier *mod = getModifier(i);
                            if (irInBuf[i].Event.KeyEvent.wVirtualKeyCode == mod->keycode)
                            mod->isActive = irInBuf[i].Event.KeyEvent.bKeyDown;
                        }
                        dowhat = KeyEventProc(0, irInBuf[i].Event.KeyEvent);
                        break;
                    }

                case MOUSE_EVENT: // mouse input
                    MouseEventProc(irInBuf[i].Event.MouseEvent);
                    break;

                case WINDOW_BUFFER_SIZE_EVENT: // scrn buf. resizing
                    ResizeEventProc( irInBuf[i].Event.WindowBufferSizeEvent );
                    break;

                case FOCUS_EVENT:  // disregard focus events

                case MENU_EVENT:   // disregard menu events
                    break;

                default:
                    ErrorExit("Unknown event type");
                    break;
            }
        }
        drawUpdate(0);
    }

    Exit();

    return 0;

}
