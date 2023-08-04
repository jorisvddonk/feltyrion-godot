#pragma once

#include "noctis-d.h"
#ifndef WITH_GODOT
#include <raylib.h>
#endif

// Definitions for noctis.cpp
#ifndef WITH_GODOT
extern RenderTexture2D temp_texture;
#endif
extern void swapBuffers();