#pragma once

bool audio_dlopen(const char *lib_name, void **result);
bool audio_dlsym(void *handle, const char *symbol, void **result);