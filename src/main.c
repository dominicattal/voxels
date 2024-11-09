#include "state.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    atexit(state_destroy);
    state_init();
    state_loop();
    return 0;
}