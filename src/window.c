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

    window->background = (struct RGBA) { 0, 0, 0, 255 };
    window->height = height;
    window->width = width;
    window->last_time = 0;
    window->is_done = 0;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(width, height, 
      SDL_WINDOW_SHOWN, &window->window, &window->renderer);
    SDL_GetRendererInfo(window->renderer, &window->renderer_info);

    return 1;
}

/*
 * Clears the renderer, setting a solid background color. Defaults to 
 * rbga(0, 0, 0, 255). This method can be called like `window:clear(r,g,b,a)'
 * to set the new default.  This method can also be passed no arguments to clear 
 * with the default.
 */
static int
lua_window_clear (lua_State *L)
{
    Window *window = lua_check_window(L, 1);
    struct RGBA bg = {
        .r = luaL_optinteger(L, 2, window->background.r),
        .g = luaL_optinteger(L, 3, window->background.g),
        .b = luaL_optinteger(L, 4, window->background.b),
        .alpha = luaL_optinteger(L, 5, window->background.alpha)
    };

    if (lua_gettop(L) == 5)
        window->background = (struct RGBA) { bg.r, bg.g, bg.b, bg.alpha };
    
    SDL_SetRenderDrawColor(window->renderer, bg.r, bg.g, bg.b, bg.alpha);
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
 * Destroy the Window from a method.
 */
static int
lua_window_quit (lua_State *L)
{
    Window *window = lua_check_window(L, 1);

    if (!window->is_done) {
        SDL_DestroyRenderer(window->renderer);
        SDL_DestroyWindow(window->window);
        SDL_Quit();
        window->is_done = 1;
    }

    return 0;
}

/*
 * Destroy the Window on garbage collection.
 */
static int
lua_window_gc (lua_State *L)
{
    return lua_window_quit(L);
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

static int
lua_window_error (lua_State *L)
{
    lua_check_window(L, 1);
    lua_pushstring(L, SDL_GetError());
    return 1;
}

void
window_push_event_motion (Window *window, lua_State *L)
{
    lua_newtable(L);

    lua_pushinteger(L, window->event.motion.x);
    lua_setfield(L, -2, "x");

    lua_pushinteger(L, window->event.motion.y);
    lua_setfield(L, -2, "y");
}

/*
 * Push the key that was part of an event.
 */
void
window_push_event_key (Window *window, lua_State *L) 
{
    switch(window->event.key.keysym.sym) {
    case SDLK_ESCAPE: case SDL_QUIT:
        lua_pushstring(L, "quit"); break; 
    case SDLK_w:
        lua_pushstring(L, "w"); break; 
    case SDLK_a:
        lua_pushstring(L, "a"); break; 
    case SDLK_s:
        lua_pushstring(L, "s"); break; 
    case SDLK_d:
        lua_pushstring(L, "d"); break; 
    case SDLK_UP:
        lua_pushstring(L, "up"); break; 
    case SDLK_LEFT:
        lua_pushstring(L, "left"); break;
    case SDLK_DOWN:
        lua_pushstring(L, "down"); break;
    case SDLK_RIGHT:
        lua_pushstring(L, "right"); break;
    case SDLK_SPACE:
        lua_pushstring(L, "space"); break;
    default:
      lua_pushstring(L, "unsupport key"); break;
    }
}

/*
 * Returns both the type of data and data itself. The data is a table which is
 * dependent on the type of data.
 *
 * if window:has_event() then
 *     e_type, e_data = window:get_event()
 *
 *     if e_type == "key_down"
 *     elseif e_type == "key_up"
 *     elseif e_type == "mouse_move"
 *     elseif e_type == "mouse_click"
 *     end
 * end
 */
static int
lua_window_get_event (lua_State *L)
{
    int no_event = 0;
    Window *window = lua_check_window(L, 1);

    if (SDL_PollEvent(&window->event)) {
        switch (window->event.type) {
        case SDL_KEYDOWN:
            lua_pushstring(L, "key_down");
            break;

        case SDL_KEYUP:
            lua_pushstring(L, "key_up");
            break;

        case SDL_MOUSEMOTION:
            lua_pushstring(L, "mouse_move");
            break;

        case SDL_MOUSEBUTTONDOWN:
            lua_pushstring(L, "mouse_click_down");
            break;

        case SDL_MOUSEBUTTONUP:
            lua_pushstring(L, "mouse_click_up");
            break;

        default:
            lua_pushstring(L, "not supported");
            break;
        }
    } else {
        lua_pushstring(L, "no event");
        no_event = 1;
    }
    
    lua_newtable(L);

    if (no_event)
        goto exit;

    switch (window->event.type) {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
        window_push_event_key(window, L);
        lua_setfield(L, -2, "key");
        break;

    case SDL_MOUSEMOTION:
        window_push_event_motion(window, L);
        lua_setfield(L, -2, "location");
        break;

    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        break;

    default:
        break;
    }

exit:
    return 2;
}

/*
 * TODO: Make this function accept a callback to be done every N times per
 * second.
 *
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

    if (current_time - window->last_time > (1000/n_times)) {
        window->last_time = current_time;
        lua_pushboolean(L, 1);
    } else {
        lua_pushboolean(L, 0);
    }

    return 1;
}

static const luaL_Reg window_methods[] = {
    {"clear",     lua_window_clear},
    {"draw",      lua_window_draw},
    {"set_color", lua_set_color},
    {"render",    lua_window_render},
    {"per_second",lua_window_per_second},
    {"get_event", lua_window_get_event},
    {"error",     lua_window_error},
    {"quit",      lua_window_quit},
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
