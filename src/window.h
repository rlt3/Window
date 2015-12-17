#ifndef LUA_WINDOW_H
#define LUA_WINDOW_H

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <SDL2/SDL.h>

#define WINDOW_LIB "Window"

struct RGBA {
    int r;
    int g;
    int b;
    int alpha;
};

typedef struct Window {
    struct RGBA background;
    int height;
    int width;
    uint32_t last_time;
    SDL_Renderer     *renderer;
    SDL_Window       *window;
    SDL_RendererInfo  renderer_info;
    SDL_Event         event;
} Window;

#endif
