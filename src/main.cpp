#include "bridge.h"
#include <stdio.h>

using UATBridge::Bridge;

int main (int argc, char** argv)
{
    printf("Starting %s %s\n", Bridge::NAME, Bridge::VERSION);
    {
        Bridge bridge;
        if (!bridge.run()) {
            fprintf(stderr, "Error\n");
            return 1;
        }
    }
    printf("Good bye\n");
    return 0;
}
