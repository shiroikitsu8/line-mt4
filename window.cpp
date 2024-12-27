#include "window.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <thread>
#include "gl.h"
#include "shared/input.h"
#include "touch.h"

bool window_ready = false;
void window_thread()
{
    SDL_Window *window = SDL_CreateWindow("NU Next Generation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1360, 768, 0);
    if (!window) {
        printf("Error creating window\n");
        return;
    }

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window, &wmInfo);

    HWND hWnd = wmInfo.info.win.window;

    gl_init(hWnd);

    window_ready = true;
    while (true)
    {
        MSG msg = {};
        while (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        SDL_Event event;
        while (SDL_PollEvent(&event) != 0)
        {
            input_sdl(event);

            switch (event.type) {
                case SDL_MOUSEBUTTONUP:
                case SDL_MOUSEBUTTONDOWN: {
                    bool down = event.type == SDL_MOUSEBUTTONDOWN;

                    touch_update(down, event.button.x, event.button.y);
                    break;
                }
            }
        }

        Sleep(8);
    }
}

void window_init()
{
    std::thread t(window_thread);
    t.detach();
    while (!window_ready)
    {
        Sleep(10);
    }
}