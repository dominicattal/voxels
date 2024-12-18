#include "state.h"
#include "util/env.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    env_init();
    srand(time(NULL));
    atexit(state_destroy);
    state_init();
    state_loop();
    return 0;
}