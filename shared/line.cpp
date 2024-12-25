#include "line.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void *(__cdecl *real_DlOpen)(const char *name);
void *(__cdecl *real_DlSym)(void *handle, const char *symbol);
void *(__cdecl *real_Hook)(void *address, void *target, void **old);
void *(__cdecl *real_GetModuleByName)(const char *name);
void *(__cdecl *real_GetModuleStart)(void *module);
uint32_t(__cdecl *real_GetModuleSize)(void *module);
void *(__cdecl *real_GetModuleByBaseHandle)(void *handle);
void *(__cdecl *real_ResolveStub)(const char *name);

void Line::Init(void **func_tables)
{
    *(void **)&real_DlOpen = func_tables[0];
    *(void **)&real_DlSym = func_tables[1];
    *(void **)&real_Hook = func_tables[2];
    *(void **)&real_GetModuleByName = func_tables[3];
    *(void **)&real_GetModuleStart = func_tables[4];
    *(void **)&real_GetModuleSize = func_tables[5];
    *(void **)&real_GetModuleByBaseHandle = func_tables[6];
    *(void **)&real_ResolveStub = func_tables[7];
}

void *Line::DlOpen(const char *name)
{
    return real_DlOpen(name);
}
void *Line::DlSym(void *handle, const char *symbol)
{
    return real_DlSym(handle, symbol);
}

void Line::Hook(void *address, void *target, void **old)
{
    real_Hook(address, target, old);
}

void Line::Patch(void *address, std::vector<uint8_t> data)
{
    memcpy(address, data.data(), data.size());
}

void Line::PatchString(void *address, const char *data)
{
    memcpy(address, data, strlen(data) + 1);
}

void Line::Nop(void *address, uint32_t length)
{
    memset(address, 0x90, length);
}

#define PATTERN_UNKNOWN 256

// Example: FF ?? 00 00 00 or FF ? 00 00 00
std::vector<uint16_t> parsePattern(const char *pattern)
{
    strlen(pattern);

    std::vector<uint16_t> out = {};

    size_t length = strlen(pattern);

    bool lastUnknown = false;
    for (size_t i = 0; i < length; i++)
    {
        if (pattern[i] == '?')
        {
            if (lastUnknown)
                continue;
            lastUnknown = true;

            out.push_back(PATTERN_UNKNOWN);
        }
        else
        {
            lastUnknown = false;

            if (pattern[i] == ' ')
                continue;

            if (length - i < 2)
                break;

            char *p = (char *)&pattern[i];
            char *p2 = (char *)&pattern[i + 1];
            out.push_back(strtol(p, &p2, 16));
            i++;
        }
    }
    return out;
}

void *Line::Signature(const char *pattern, int offset)
{
    std::vector<uint16_t> bytes = parsePattern(pattern);

    void *module = real_GetModuleByBaseHandle(NULL);
    void *address = real_GetModuleStart(module);
    uint32_t size = real_GetModuleSize(module);

    if (bytes.empty())
        return NULL;

    uint8_t *ptr = (uint8_t *)address;
    for (size_t i = 0; i < size - bytes.size(); i++)
    {
        bool found = true;
        for (size_t j = 0; j < bytes.size(); j++)
        {
            if (bytes[j] == PATTERN_UNKNOWN)
                continue;
            if (ptr[i + j] == bytes[j])
                continue;
            found = false;
            break;
        }
        if (!found)
            continue;
        return &ptr[i + offset];
    }
    return NULL;
}

void *Line::ResolveCall(void *address)
{
    uintptr_t resolved = 0;

    uint8_t *data = (uint8_t *)address;
    switch (data[0])
    {
    case 0xE8: // rel call
    case 0xE9: // rel jmp
    {
        int32_t offset = *(int32_t *)&data[1];

        resolved = (uintptr_t)address + 5 + offset;
    }
    }
    return (void *)resolved;
}

void *Line::ResolveStub(const char *name)
{
    return real_ResolveStub(name);
}