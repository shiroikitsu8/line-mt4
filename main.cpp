#include <cstdio>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdarg.h>
#include <pthread.h>
#include <string.h>

#include <SDL2/SDL.h>

#include "audio.h"
#include "bana.h"
#include "compress.h"
#include "gl.h"
#include "hasp.h"
#include "input.h"
#include "mq.h"
#include "net.h"
#include "window.h"
#include "yac.h"

#include "shared/config.h"
#include "shared/line.h"

#include <glad/gl.h>
#include "touch.h"

#define DllExport __declspec(dllexport)

int jmp_fork()
{
    return 1;
}

void Sys_LogMes(int type, char *msg, ...)
{
    switch (type)
    {
    case 1:
    {
        printf("[INF] ");
        break;
    }
    case 2:
    {
        printf("[WRN] ");
        break;
    }
    case 3:
    {
        printf("[SYS] ");
        break;
    }
    case 4:
    {
        printf("[ERR] ");
        break;
    }
    }

    va_list va;
    va_start(va, msg);
    vprintf(msg, va);
    va_end(va);
}

int althman_get_my_pid()
{
    return (int)pthread_self();
}

char *(__cdecl *old_getenv)(const char *name);
char *jmp_getenv(const char *name)
{
    if (strcmp(name, "__NU_SCREEN_WINDOWED") == 0)
        return (char *)"1";
    return old_getenv(name);
}

int send_command(int a1, int a2, int a3, int a4, char *)
{
    return 0;
}

int lumen_debug_printfmt(const char *format, ...)
{
    printf("[LMN] ");

    va_list va;
    va_start(va, format);
    int len = vprintf(format, va);
    va_end(va);
    return len;
}

void(__cdecl *old_glTexParameteri)(GLenum target, GLenum pname, GLenum param);
void jmp_glTexParameteri(GLenum target, GLenum pname, GLenum param)
{
    if (param == GL_CLAMP)
        param = GL_CLAMP_TO_EDGE;
    old_glTexParameteri(target, pname, param);
}

void *lib_cg_so = NULL;
void *lib_cgGL_so = NULL;
void *lib_curl = NULL;

void(__cdecl *old_std_string_assign)(void *a1, void *a2);
void jmp_std_string_assign(void *a1, void *a2)
{
    if (!a2)
        return;
    // check for empty string
    if (*(int *)a2 == 0)
        return;

    old_std_string_assign(a1, a2);
}

extern "C"
{
    DllExport void OnInitialize(int version, void **func_tables)
    {
        printf("Plugin OnInitialize\n");

        Line::Init(func_tables);

        config.load();

        SDL_SetMainReady();

        SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS4, "1");
        SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS4_RUMBLE, "1");
        SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5, "1");
        SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5_RUMBLE, "1");

        if (config.inputBackground)
        {
            SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
        }
        else
        {
            SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "0");
        }
        SDL_SetHint(SDL_HINT_JOYSTICK_RAWINPUT, "0");

        if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_HAPTIC) != 0)
        {
            printf("Can't initialize SDL2: %s\n", SDL_GetError());
            return;
        }

        SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");

        SDL_GameControllerEventState(SDL_ENABLE);
        SDL_JoystickEventState(SDL_ENABLE);

        printf("Initialized SDL2\n");

        window_init();

        lib_curl = dlopen("libcurl.dll", RTLD_NOW);
        if (!lib_curl)
            printf("can't load libcurl.dll\n");

        lib_cg_so = dlopen("cg.dll", RTLD_NOW);
        if (!lib_cg_so)
            printf("can't load cg.dll\n");

        lib_cgGL_so = dlopen("cgGL.dll", RTLD_NOW);
        if (!lib_cgGL_so)
            printf("can't load cgGL.dll\n");
    }

    DllExport void OnPreExecute(const char *lib_name, uintptr_t base_address)
    {
        printf("Plugin OnPreExecute %s\n", lib_name);

        if (strcmp(lib_name, "WMN4r") == 0)
        {
            if (config.bana)
                bana_init();
            compress_init();
            hasp_init(config.terminal ? "267611069420" : "267610069420");
            if (config.input)
                input_init();
            net_init();
            if (config.touch)
                touch_init();

            Line::Hook((void *)0x809ddb0, (void *)Sys_LogMes);
            Line::Hook((void *)0x8058018, (void *)jmp_fork);
            Line::Hook((void *)0x8057428, (void *)althman_get_my_pid);
            Line::Hook((void *)0x8058e38, (void *)jmp_getenv, (void**)&old_getenv);

            Line::Hook((void *)0x8059228, (void *)jmp_glTexParameteri, (void **)&old_glTexParameteri);

            // dont throw on sound engine initialize
            Line::Patch((void *)0x83d9470, {
                                               0xc6, 0x05, 0x18, 0xfe, 0x21, 0x09, 0x01, // Set initialize global to true
                                               0xb8, 0x01, 0x00, 0x00, 0x00,            // MOV EAX,0x1 (set return value)
                                               0x8b, 0x5d, 0xf4,                        // MOV EBX,dword ptr [EBP + local_10]
                                               0x8b, 0x75, 0xf8,                        // MOV ESI,dword ptr [EBP + local_c]
                                               0x8b, 0x7d, 0xfc,                        // MOV EDI,dword ptr [EBP + local_8]
                                               0x89, 0xec,                              // MOV ESP,EBP
                                               0x5d,                                    // POP
                                               0xc3                                     // RET
                                           });

            Line::Hook((void *)0x89e5c80, (void *)send_command);

            // mucha
            Line::PatchString((void *)0x8c11004, "mucha.local");
            Line::PatchString((void *)0x8c11028, "mucha.local"); // cn

            // disable ssl verify
            Line::Patch((void *)0x8ae1f20, {0x90, 0x90, 0x90}); // test byte ptr [edx],01 nop

            Line::Hook((void *)0x8aa1890, (void *)lumen_debug_printfmt);

            if (!config.terminalProxy)
            {
                if (config.terminal)
                {
                    // Disable proxy because it locks the game
                    Line::Patch((void *)0x809fa23, {0x90,
                                                    0x90,
                                                    0x90,
                                                    0x90,
                                                    0x90});
                    // dont set rpc to 0.0.0.0
                    Line::Patch((void *)0x80a0c88, {0x90, 0x90, 0x90, 0x90, 0x90, 0x90});
                }

                // disable using terminal proxy
                Line::Patch((void *)0x80a0c99, {0x90, 0x90, 0x90, 0x90, 0x90, 0x90});
            }

            Line::Hook((void *)0x80593a8, (void *)jmp_std_string_assign, (void **)&old_std_string_assign);
            
            if (config.yac)
                yac_init();

            void *lib_gs = dlopen("gspatch.dll", RTLD_NOW);
            if (!lib_gs)
                printf("can't load gspatch.dll\n");
        }
    }

    DllExport bool OnDlOpen(const char *lib_name, void **result)
    {
        printf("OnDlOpen %s\n", lib_name);

        if (audio_dlopen(lib_name, result))
            return true;
        if (input_dlopen(lib_name, result))
            return true;

        *result = NULL;
        return true;
    }

    DllExport bool OnDlSym(void *handle, const char *symbol, void **result)
    {
        printf("OnDlSym %s\n", symbol);

        if (audio_dlsym(handle, symbol, result))
            return true;
        if (input_dlsym(handle, symbol, result))
            return true;

        return false;
    }

    DllExport bool OnResolveSymbol(const char *symbol, void **result)
    {
        if (mq_resolve(symbol, result))
            return true;

        if (strstr(symbol, "curl_") == symbol)
        {
            *result = (void *)dlsym(lib_curl, symbol);
            return true;
        }

        if (strstr(symbol, "cg") == symbol)
        {
            void *res = (void *)dlsym(lib_cg_so, symbol);
            if (res)
            {
                *result = res;
                return true;
            }

            res = (void *)dlsym(lib_cgGL_so, symbol);
            if (res)
            {
                *result = res;
                return true;
            }
        }

        void *res = gl_resolve(symbol);
        if (res)
        {
            *result = res;
            return true;
        }

        return false;
    }

    // Skibidi rizz fix
    int __cdecl __mingw_vsscanf(const char *_Str, const char *Format, va_list argp)
    {
        return vsscanf(_Str, Format, argp);
    }

    int __cdecl __mingw_vsnprintf(char *_DstBuf, size_t _MaxCount, const char *_Format,
                                  va_list _ArgList)
    {
        return vsnprintf(_DstBuf, _MaxCount, _Format, _ArgList);
    }

    int __cdecl __mingw_vsprintf(char *a1, const char *a2, va_list a3)
    {
        return vsprintf(a1, a2, a3);
    }
};
