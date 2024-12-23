#include "env.h"
#include <stdio.h>
#include <stdlib.h>

u32   ENV_VAR1;
f32   ENV_VAR2;
char* ENV_VAR3;
bool  ENV_EXPORT_TEXTURE_ATLASES;

void env_init(void)
{
    ENV_VAR1 = (getenv("VAR1")) ? atoi(getenv("VAR1")) : -1;
    ENV_VAR2 = (getenv("VAR2")) ? atof(getenv("VAR2")) : -1;
    ENV_VAR3 = (getenv("VAR3")) ? getenv("VAR3") : "";
    ENV_EXPORT_TEXTURE_ATLASES = (getenv("EXPORT_TEXTURE_ATLASES")) ? atoi(getenv("EXPORT_TEXTURE_ATLASES")) : 0;
}