#include "compress.h"
#include "shared/line.h"
#include <cstdio>
#include <string.h>
#include <gzip/decompress.hpp>

#pragma push(pack, 1)
struct Sys_LoadRequest
{
    char pad[276];
    void *user; // 276
    void *heap; // 280
    int align;  // 284
};

#pragma pop(pack)

typedef void *(*t_nuMemoryAllocEx)(int type, size_t align, size_t size);
t_nuMemoryAllocEx nuMemoryAllocEx = NULL;

typedef void (*t_nuMemoryFreeEx)(void *data);
t_nuMemoryFreeEx nuMemoryFreeEx = NULL;

typedef void *(*t_GCL_HeapNwAlloc)(size_t size, size_t align, void *heap);
t_GCL_HeapNwAlloc GCL_HeapNwAlloc = NULL;

typedef void (*t_GCL_HeapRelease)(void *data, void *heap);
t_GCL_HeapRelease GCL_HeapRelease = NULL;

struct UserWrapper
{
    void *user;
    void *heap;
    int align;
};

int(__cdecl *old_Sys_LoadRequest_stdLoad)(Sys_LoadRequest *);
int jmp_Sys_LoadRequest_stdLoad(Sys_LoadRequest *request)
{
    UserWrapper *wrapper = new UserWrapper;
    wrapper->user = request->user;
    wrapper->heap = request->heap;
    wrapper->align = request->align;

    request->user = wrapper;
    return old_Sys_LoadRequest_stdLoad(request);
}

void(__cdecl *old_Sys_fileDescCallback)(int result, void *buff, int size, void *user);
void jmp_Sys_fileDescCallback(int result, void *buff, int size, void *user)
{
    UserWrapper *wrapper = (UserWrapper *)user;
    if (result != 0 || size < 1)
    {
        old_Sys_fileDescCallback(result, buff, size, wrapper->user);
        delete wrapper;
        return;
    }

    if (size >= 2 && *(uint16_t *)buff == 35615)
    { // gzip header
        try
        {
            std::string decompressed_data = gzip::decompress((char *)buff, size);
            printf("Decompressed %d -> %d, heap=%d\n", size, decompressed_data.size(), wrapper->heap != NULL);

            if (wrapper->heap)
            {
                GCL_HeapRelease(buff, wrapper->heap);
            }
            else
            {
                nuMemoryFreeEx(buff);
            }

            if (decompressed_data.size() < 1)
            {                                                          // empty
                old_Sys_fileDescCallback(result, 0, 0, wrapper->user); // no size OK!
                delete wrapper;
                return;
            }

            int decSize = decompressed_data.size();
            void *newData;

            int decSizeAlign = wrapper->align * ((decSize + wrapper->align - 1) / wrapper->align);
            if (decSize != decSizeAlign)
                printf("align %d -> %d\n", decSize, decSizeAlign);
            if (wrapper->heap)
            {
                newData = GCL_HeapNwAlloc(decSizeAlign, wrapper->align, wrapper->heap);
                if (!newData)
                    printf("cant allocate %d of data (GCL::HeapNwAlloc)\n", decSizeAlign);
            }
            else
            {
                newData = nuMemoryAllocEx(0, wrapper->align, decSizeAlign);
                if (!newData)
                    printf("cant allocate %d of data (nuMemoryAllocEx)\n", decSizeAlign);
            }

            memcpy(newData, decompressed_data.data(), decSize);
            decompressed_data.clear(); // free up
            old_Sys_fileDescCallback(result, newData, decSize, wrapper->user);
            delete wrapper;
            return;
        }
        catch (...)
        {
            printf("exception when decompressing gzip\n");
        }
    }
    old_Sys_fileDescCallback(result, buff, size, wrapper->user);
    delete wrapper;
}

void compress_init()
{
    nuMemoryAllocEx = (t_nuMemoryAllocEx)0x89955c0;
    nuMemoryFreeEx = (t_nuMemoryFreeEx)0x8995620;
    GCL_HeapNwAlloc = (t_GCL_HeapNwAlloc)0x805e4f0;
    GCL_HeapRelease = (t_GCL_HeapRelease)0x805e710;

    Line::Hook((void *)0x809bc40, (void *)jmp_Sys_LoadRequest_stdLoad, (void **)&old_Sys_LoadRequest_stdLoad);
    Line::Hook((void *)0x809b480, (void *)jmp_Sys_fileDescCallback, (void **)&old_Sys_fileDescCallback);
}