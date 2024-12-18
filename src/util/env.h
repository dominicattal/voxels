#ifndef ENV_H
#define ENV_H

#include "type.h"

extern u32   ENV_VAR1;
extern f32   ENV_VAR2;
extern char* ENV_VAR3;

void env_init(void);

#endif