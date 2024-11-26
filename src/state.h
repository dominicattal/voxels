#ifndef STATE_H
#define STATE_H

#include "renderer/renderer.h"
#include "window/window.h"
#include "gui/gui.h"
#include "font/font.h"
#include "audio/audio.h"

void state_init(void);
void state_loop(void);
void state_destroy(void);

#endif