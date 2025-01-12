#include "env.h"
#include <stdio.h>
#include <stdlib.h>

bool ENV_EXPORT_TEXTURE_ATLASES;
bool ENV_SHOW_GL_NOTIFICATIONS;

void env_init(void)
{
    ENV_EXPORT_TEXTURE_ATLASES = (getenv("EXPORT_TEXTURE_ATLASES")) ? atoi(getenv("EXPORT_TEXTURE_ATLASES")) : 0;
    ENV_SHOW_GL_NOTIFICATIONS = (getenv("SHOW_GL_NOTIFICATIONS")) ? atoi(getenv("SHOW_GL_NOTIFICATIONS")) : 0;
}