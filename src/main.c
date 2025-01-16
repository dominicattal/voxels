#include "state.h"
#include "util/util.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    env_init();
    atexit(state_destroy);
    state_init();
    state_loop();
    return 0;
}