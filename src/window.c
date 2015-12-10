#include <stdio.h>
#include "window.h"

Window *
lua_check_window (lua_State *L, int index)
{
    return (Window *) luaL_checkudata(L, index, WINDOW_LIB);
}

/*
 * Create a new Window given the height and width: new(height, width)
 */
static int
lua_window_new (lua_State *L)
{
    int height = luaL_checkint(L, 1);
    int width = luaL_checkint(L, 2);

    Window *window = lua_newuserdata(L, sizeof(*window));
    luaL_getmetatable(L, WINDOW_LIB);
    lua_setmetatable(L, -2);

    window->height = height;
    window->width = width;
    window->last_time = 0;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(width, height, 
      SDL_WINDOW_SHOWN, &window->window, &window->renderer);
    SDL_GetRendererInfo(window->renderer, &window->renderer_info);

    return 1;
}

/*
 * Clears the window with the given color as a table in the form  {r, g, b, a}.
 * If no table is given, it assumes {0, 0, 0, 255}
 */
static int
lua_window_clear (lua_State *L)
{
    Window *window = lua_check_window(L, 1);
    int r = 0;
    int g = 0;
    int b = 0;
    int alpha = 255;
    
    SDL_SetRenderDrawColor(window->renderer, r, g, b, alpha);
    SDL_RenderClear(window->renderer);

    return 0;
}

/*
 * Render the Window.
 */
static int
lua_window_render (lua_State *L)
{
    Window *window = lua_check_window(L, 1);
    SDL_RenderPresent(window->renderer);
    return 0;
}

/*
 * Destroy the Window on garbage collection.
 */
static int
lua_window_gc (lua_State *L)
{
    Window *window = lua_check_window(L, 1);
    SDL_DestroyRenderer(window->renderer);
    SDL_DestroyWindow(window->window);
    SDL_Quit();
    return 0;
}

static int
lua_set_color (lua_State *L)
{
    Window *window = lua_check_window(L, 1);
    int r = luaL_checkint(L, 2);
    int g = luaL_checkint(L, 3);
    int b = luaL_checkint(L, 4);
    int alpha = luaL_checkint(L, 5);
    SDL_SetRenderDrawColor(window->renderer, r, g, b, alpha);
    return 0;
}

static int
lua_window_draw (lua_State *L)
{
    Window *window = lua_check_window(L, 1);
    int x = luaL_checkint(L, 2);
    int y = luaL_checkint(L, 3);
    int w = luaL_checkint(L, 4);
    int h = luaL_checkint(L, 5);
    SDL_Rect r = { x, y, w, h };
    SDL_RenderFillRect(window->renderer, &r); 
    return 0;
}

/*
 * if window:per_second(30) then do_stuff() end
 *
 * Is true up to N times per second. False otherwise.
 */
static int
lua_window_per_second (lua_State *L)
{
    Window *window = lua_check_window(L, 1);
    const int n_times = luaL_checkint(L, 2);
    uint32_t current_time = SDL_GetTicks();

    if (current_time - window->last_time > (1000/n_times))
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);

    return 1;
}

static const luaL_Reg window_methods[] = {
    {"clear",     lua_window_clear},
    {"draw",      lua_window_draw},
    {"set_color", lua_set_color},
    {"render",    lua_window_render},
    {"per_second",lua_window_per_second},
    {"__gc",      lua_window_gc},
    { NULL, NULL }
};

int
luaopen_Window (lua_State *L)
{
    /* create metatable */
    luaL_newmetatable(L, WINDOW_LIB);

    /* metatable.__index = metatable */
    lua_pushvalue(L, -1);
    lua_setfield(L, -1, "__index");

    /* register methods */
    luaL_setfuncs(L, window_methods, 0);

    lua_newtable(L);
    lua_pushcfunction(L, lua_window_new);
    lua_setfield(L, -2, "new");

    return 1;
}
