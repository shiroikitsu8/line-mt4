#include "input.h"
#include <string.h>
#include <unordered_map>
#include "shared/line.h"
#include "shared/input.h"
#include "shared/config.h"

#pragma pack(push, 1)
#define TESTSWITCH_TEST 0x80

#define SWITCH_INTERRUPT 0x0001
#define SWITCH_VIEW_CHANGE 0x0002
#define SWITCH_GEAR_RIGHT 0x0010
#define SWITCH_GEAR_LEFT 0x0020
#define SWITCH_GEAR_DOWN 0x0040
#define SWITCH_GEAR_TOP 0x0080
#define SWITCH_TERMINAL 0x0100
#define SWITCH_TEST_ENTER 0x0200
#define SWITCH_TEST_DOWN 0x1000
#define SWITCH_TEST_UP 0x2000
#define SWITCH_SERVICE 0x4000

// size=28
struct Sys_Device_N2JVIO_COIN
{
    uint16_t coin;       // 0
    uint16_t inc;        // 2
    uint16_t dec;        // 4
    uint8_t status;      // 6 (1 = jammed)
    uint8_t slot_info;   // 7 ( 1 = COIN, 2 = CARD)
    uint32_t mode;       // 8
    uint32_t retry;      // 12
    uint16_t cid;        // 16
    uint16_t mech_coin;  // 18
    uint16_t mech_rate;  // 20
    uint16_t mech_point; // 22
    uint16_t mech_dec;   // 24
    char pad[2];         // 26
};

// size=12
struct Sys_Device_N2JVIO_SERV
{
    uint16_t service; // 0
    uint16_t inc;     // 2
    uint16_t dec;     // 4
    uint16_t timer;   // 6
    uint32_t mode;    // 8
};

// size=308
struct Sys_Device_JVIODATA
{
    uint8_t testsw;                    // 0
    uint8_t dipsw;                     // 1
    uint8_t key;                       // 2
    char pad[1];                       // 3
    uint16_t sw[16];                   // 4
    Sys_Device_N2JVIO_COIN coin[4];    // 36
    Sys_Device_N2JVIO_SERV service[4]; // 148
    uint8_t gout[30];                  // 196
    uint16_t adout[8];                 // 226
    uint16_t adin[16];                 // 242
    uint16_t gsw[8];                   // 274
    uint8_t serialn;                   // 290
    uint8_t serial[16];                // 291
    char pad2[1];                      // 307

    bool get_test()
    {
        return (this->testsw & TESTSWITCH_TEST) != 0;
    }

    void set_test(bool state)
    {
        if (state)
            this->testsw |= TESTSWITCH_TEST;
        else
            this->testsw &= ~TESTSWITCH_TEST;
    }

#define DEF_SWITCH_GET_SET(name, value)    \
    bool get_##name()                      \
    {                                      \
        return (this->sw[0] & value) != 0; \
    }                                      \
    void set_##name(bool state)            \
    {                                      \
        if (state)                         \
        {                                  \
            this->sw[0] |= value;          \
        }                                  \
        else                               \
        {                                  \
            this->sw[0] &= ~value;         \
        }                                  \
    }
    DEF_SWITCH_GET_SET(interrupt, SWITCH_INTERRUPT)
    DEF_SWITCH_GET_SET(view_change, SWITCH_VIEW_CHANGE)
    DEF_SWITCH_GET_SET(gear_right, SWITCH_GEAR_RIGHT)
    DEF_SWITCH_GET_SET(gear_left, SWITCH_GEAR_LEFT)
    DEF_SWITCH_GET_SET(gear_down, SWITCH_GEAR_DOWN)
    DEF_SWITCH_GET_SET(gear_top, SWITCH_GEAR_TOP)
    DEF_SWITCH_GET_SET(terminal, SWITCH_TERMINAL)
    DEF_SWITCH_GET_SET(test_enter, SWITCH_TEST_ENTER)
    DEF_SWITCH_GET_SET(test_down, SWITCH_TEST_DOWN)
    DEF_SWITCH_GET_SET(test_up, SWITCH_TEST_UP)
    DEF_SWITCH_GET_SET(service, SWITCH_SERVICE)

    void set_wheel(uint16_t value)
    {
        this->adin[0] = value;
    }

    void set_gas(uint16_t value)
    {
        this->adin[1] = value;
    }

    void set_brakes(uint16_t value)
    {
        this->adin[2] = value;
    }

    int get_gear()
    {
        bool right = this->get_gear_right();
        bool left = this->get_gear_left();
        bool down = this->get_gear_down();
        bool top = this->get_gear_top();

        if (top && right)
            return 5;
        if (down && right)
            return 6;
        if (top && left)
            return 1;
        if (down && left)
            return 2;
        if (top)
            return 3;
        if (down)
            return 4;
        return 0;
    }

    void set_gear(int gear)
    {
        bool right = false, left = false, down = false, top = false;

        switch (gear)
        {
        case 0:
            break;
        case 1:
        {
            left = true;
            top = true;
            break;
        }
        case 2:
        {
            left = true;
            down = true;
            break;
        }
        case 3:
        {
            top = true;
            break;
        }
        case 4:
        {
            down = true;
            break;
        }
        case 5:
        {
            right = true;
            top = true;
            break;
        }
        case 6:
        {
            right = true;
            down = true;
            break;
        }
        }

        this->set_gear_right(right);
        this->set_gear_left(left);
        this->set_gear_down(down);
        this->set_gear_top(top);
    }
};

struct Sys_Device_JvioControl
{
    char pad[12];
    Sys_Device_JVIODATA jvioData[2]; // 12
};

struct spJvioControl
{
    Sys_Device_JvioControl *px;
};

#pragma pack(pop)

spJvioControl *Sys_Device_JvioControl_s_spInstance;

bool JvioControl_IsBooted()
{
    return true;
}

bool last_coin = false;
void JvioControl_Update()
{
    if (!Sys_Device_JvioControl_s_spInstance->px)
        return;

    Sys_Device_JVIODATA *jvioData = &Sys_Device_JvioControl_s_spInstance->px->jvioData[1];

    jvioData->set_test(get_test());
    jvioData->set_test_up(get_test_up());
    jvioData->set_test_down(get_test_down());
    jvioData->set_test_enter(get_test_enter());
    jvioData->set_gas(get_analog_gas());
    jvioData->set_brakes(get_analog_brakes());
    jvioData->set_view_change(get_view_change());
    jvioData->set_interrupt(get_interrupt());
    jvioData->set_wheel(get_analog_wheel());
    jvioData->set_gear(get_gear());
    jvioData->set_terminal(config.terminal);
    jvioData->set_service(get_service());

    if (get_coin())
    {
        if (!last_coin)
        {
            last_coin = true;
            jvioData->coin[0].coin++;
        }
    }
    else
    {
        last_coin = false;
    }
}

int Sys_Device_JvioControl_JvioGetCoinNow(int node, int channel)
{
    if (!Sys_Device_JvioControl_s_spInstance->px)
        return 0;

    Sys_Device_JVIODATA *jvioData = &Sys_Device_JvioControl_s_spInstance->px->jvioData[node];
    Sys_Device_N2JVIO_COIN *coin = &jvioData->coin[channel];

    return coin->coin + coin->inc - coin->dec;
}

int Sys_Device_JvioControl_JvioDecCoin(int node, int channel, int count)
{
    if (!Sys_Device_JvioControl_s_spInstance->px)
        return -1;

    Sys_Device_JVIODATA *jvioData = &Sys_Device_JvioControl_s_spInstance->px->jvioData[node];
    Sys_Device_N2JVIO_COIN *coin = &jvioData->coin[channel];

    coin->coin -= count;
    return 1;
}

bool JvioControl_IsError()
{
    return false;
}

void JvioThread(void *a1)
{
}

char *Sys_Device_n2JvioGetIoMonitor(int node, int line, char *buf, size_t bufSize)
{

    const char *replacement = NULL;
    switch (node)
    {
    case 0:
    {
        switch (line)
        {
        case 0:
        {
            replacement = "INDONESIA, ITS NOT JUST A INDONESIA";
            break;
        }
        case 1:
        {
            replacement = "INDONESIA IS A BRAND";
            break;
        }
        }
        break;
    }
    case 1:
    {
        switch (line)
        {
        case 0:
        {
            replacement = "INDONESIA IS THE MAN OF THE SMART PEOPLE";
            break;
        }
        case 1:
        {
            replacement = "THE PEOPLE INDONESIA THEY ARE VERY SMART";
            break;
        }
        case 2:
        {
            replacement = "I LOVE INDONESIA";
            break;
        }
        }
        break;
    }
    }

    if (replacement)
        strncpy(buf, replacement, bufSize);
    else
        buf[0] = '\0';

    return buf;
}

int jmp_alin_init()
{
    return 0;
}

void jmp_alin_term()
{
}

int jmp_alin_pad(int a1, void *a2)
{
    return 0;
}

int jmp_alin_keyboard(int a1, void *a2)
{
    return 0;
}

int jmp_alin_mouse(int a1, void *a2)
{
    return 0;
}

std::unordered_map<const char *, void *> INPUT_FUNC_STUBS = {
    {"alin_init", (void *)jmp_alin_init},
    {"alin_term", (void *)jmp_alin_term},
    {"alin_pad", (void *)jmp_alin_pad},
    {"alin_keyboard", (void *)jmp_alin_keyboard},
    {"alin_mouse", (void *)jmp_alin_mouse}};

#define INPUT_DL_ID 521312

bool input_dlopen(const char *lib_name, void **result)
{
    if (strcmp(lib_name, "./alin.dll") == 0)
    {
        *result = (void *)INPUT_DL_ID;
        return true;
    }
    return false;
}
bool input_dlsym(void *handle, const char *symbol, void **result)
{
    if ((int)handle != INPUT_DL_ID)
        return false;
    for (auto &it : INPUT_FUNC_STUBS)
    {
        if (strcmp(it.first, symbol) == 0)
        {
            *result = it.second;
            return true;
        }
    }
    *result = NULL;
    return true;
}

void input_init()
{
    Line::Hook((void *)0x8370c00, (void *)JvioControl_IsBooted);
    Line::Hook((void *)0x83711c0, (void *)JvioControl_Update);
    Line::Hook((void *)0x8372490, (void *)Sys_Device_JvioControl_JvioGetCoinNow);
    Line::Hook((void *)0x8372600, (void *)Sys_Device_JvioControl_JvioDecCoin);
    Line::Hook((void *)0x8370c30, (void *)JvioControl_IsError);
    Line::Hook((void *)0x8375a90, (void *)JvioThread);

    Line::Hook((void *)0x8390ba0, (void *)Sys_Device_n2JvioGetIoMonitor);

    Sys_Device_JvioControl_s_spInstance = (spJvioControl *)0x921589c;
}