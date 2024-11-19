#include "state.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    srand(time(NULL));
    atexit(state_destroy);
    state_init();
    state_loop();
    return 0;
}