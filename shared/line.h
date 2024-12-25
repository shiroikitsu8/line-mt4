#pragma once
#include <stdint.h>
#include <vector>

namespace Line
{
    void Init(void **func_tables);

    void *DlOpen(const char *name);
    void *DlSym(void *handle, const char *symbol);

    void Hook(void* address, void* target, void** old = 0);
    void Patch(void* address, std::vector<uint8_t> data);
    void PatchString(void* address, const char* data);
    void Nop(void* address, uint32_t length = 1);

    void* Signature(const char* pattern, int offset = 0);
    void* ResolveCall(void* address);

    void* ResolveStub(const char* name);
}