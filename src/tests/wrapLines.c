#include "../editor.h"
#include <stdint.h>

int main(int argc, char **argv)
{
    uint8_t id = newBuffer("file");
    newlines(id, 2);
    writeToBuffer(id, 0, "what. I can hardly believe it\n", 30);
    writeToBuffer(id, 1, "nice. I can keep going and going and going\n", 43);

    for (int i = 0; i < getBufferLineCount(id); ++i) {
        printf("%s", getBufferLine(id, i));
    }
    return 0;
}
