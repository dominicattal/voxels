#include "env.h"
#include <stdio.h>
#include <stdlib.h>

bool  ENV_EXPORT_TEXTURE_ATLASES;

void env_init(void)
{
    ENV_EXPORT_TEXTURE_ATLASES = (getenv("EXPORT_TEXTURE_ATLASES")) ? atoi(getenv("EXPORT_TEXTURE_ATLASES")) : 0;
}