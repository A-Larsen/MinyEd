#include "editor.h"
#include <stdint.h>

uint8_t current_buffer_id = 0;
uint8_t buffer_count = 0;

int main(int argc, char **argv)
{     

    initConsole();
    userInfoInit();
    DWORD cNumRead, i;
    INPUT_RECORD irInBuf[128];

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

    buffer_count = argc - 1;
    for (int i = 0; i < buffer_count; ++i) {
        newBuffer(argv[i + 1]);
    }

    bool first_input = false;
    HANDLE hStdin = getSTdinHandle();

    while (true)
    {
        // Wait for the events.

        if (! ReadConsoleInput(
                hStdin,      // input buffer handle
                irInBuf,     // buffer to read into
                128,         // size of read buffer
                &cNumRead) ) // number of records read
            ErrorExit("ReadConsoleInput");

        // Dispatch the events to the appropriate handler.
        // a dowhat activity can return a id for a status to display
        notifyUpdate(current_buffer_id);

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
                        KeyEventProc(current_buffer_id, irInBuf[i].Event.KeyEvent);
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
        drawUpdate(current_buffer_id);
    }

    Exit();

    return 0;

}
