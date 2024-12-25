#include "touch.h"
#include "shared/line.h"

#pragma push(pack, 1)
struct Sys_Device_TouchIo {
    char pad[16];
    int isBoot; // 16
    char pad2[60]; // 20
    int posX; // 80
    int posY; // 84
    int touched; // 88
};
#pragma pop(pack)

bool touched = false;
int touchX = 0;
int touchY = 0;

void Sys_Device_TouchIo_update(Sys_Device_TouchIo* touchIo) {
    touchIo->isBoot = 1;
    touchIo->posX = touchX;
    touchIo->posY = touchY;
    touchIo->touched = touched;
}

void touch_update(bool state, int x, int y) {
    touched = state;
    touchX = x;
    touchY = y;
}

void touch_init() {
    Line::Hook((void *)0x837e4a0, (void*)Sys_Device_TouchIo_update);
}