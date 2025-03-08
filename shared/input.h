#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <toml.hpp>

enum class InputKeybindType
{
    KEYBOARD = 0,
    CONTROLLER_BUTTON,
    CONTROLLER_AXIS,
};

enum class InputType
{
    GAS = 0,
    BRAKES,
    WHEEL,
    
    WHEEL_LEFT,
    WHEEL_RIGHT,

    VIEW_CHANGE,
    INTERRUPT,

    TEST_UP,
    TEST_DOWN,
    TEST_ENTER,

    TEST,

    SHIFT_UP,
    SHIFT_DOWN,

    GEAR_ONE,
    GEAR_TWO,
    GEAR_THREE,
    GEAR_FOUR,
    GEAR_FIVE,
    GEAR_SIX,
    GEAR_NEUTRAL,

    CARD,
    
    SERVICE,

    COIN
};

struct InputKeybind
{
    InputKeybindType type;
    union
    {
        SDL_Keycode keyboard;
        SDL_GameControllerButton controllerButton;
        SDL_GameControllerAxis controllerAxis;
        int joystickButton;
        int joystickAxis;
    };
    bool controllerAxisFull = false;
    bool controllerAxisReversed = false;
    bool controllerAxisReversedHalf = false;
};

struct Input
{
    const char *name;
    const char *full_name;
    InputType type;
    std::vector<InputKeybindType> keybindsTypes;
    std::vector<InputKeybind> keybinds = {};
};

struct InputEvent
{
    Input* input;
    InputKeybind* keybind;

    union {
        bool keyboardDown;
        bool controllerButtonDown;
        int controllerAxis;
    };
};

extern std::vector<Input> inputs;

uint16_t get_analog_gas();
uint16_t get_analog_brakes();
uint16_t get_analog_wheel();

bool get_view_change();
bool get_interrupt();

bool get_test_up();
bool get_test_down();
bool get_test_enter();

bool get_test();

int get_gear();

bool get_card();

bool get_service();

bool get_coin();

void input_sdl(const SDL_Event& event);

Input* input_ext_get_adding_keybind();
void input_ext_set_adding_keybind(Input* input);

void input_config_read(toml::table config);
toml::table input_config_write();