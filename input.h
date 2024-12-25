#pragma once

bool input_dlopen(const char *lib_name, void **result);
bool input_dlsym(void *handle, const char *symbol, void **result);

void input_init();