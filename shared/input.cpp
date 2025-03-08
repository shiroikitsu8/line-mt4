#include "input.h"
#include <thread>
#include <chrono>

float gas_value = 0;
float brakes_value = 0;
float wheel_value = 0.5f; // 0 - 1

bool wheel_left = false;
bool wheel_right = false;

bool view_change_value = false;
bool interrupt_value = false;

bool test_up_value = false;
bool test_down_value = false;
bool test_enter_value = false;

bool test_value = false;

int gear_value = 0;

bool card_value = false;

bool service_value = false;

bool coin_value = false;

// For controller fix
bool keyShiftPressed = false;
int keyTestPressed = 0;

std::vector<Input> inputs = {
    {"gas", "Gas", InputType::GAS, {InputKeybindType::KEYBOARD, InputKeybindType::CONTROLLER_AXIS}},
    {"brakes", "Brakes", InputType::BRAKES, {InputKeybindType::KEYBOARD, InputKeybindType::CONTROLLER_AXIS}},
    {"wheel", "Wheel", InputType::WHEEL, {InputKeybindType::CONTROLLER_AXIS}},
    {"wheel_left", "Wheel Left", InputType::WHEEL_LEFT, {InputKeybindType::KEYBOARD, InputKeybindType::CONTROLLER_BUTTON}},
    {"wheel_right", "Wheel Right", InputType::WHEEL_RIGHT, {InputKeybindType::KEYBOARD, InputKeybindType::CONTROLLER_BUTTON}},

    {"view_change", "View Change", InputType::VIEW_CHANGE, {InputKeybindType::KEYBOARD, InputKeybindType::CONTROLLER_BUTTON}},
    {"interrupt", "Interrupt", InputType::INTERRUPT, {InputKeybindType::KEYBOARD, InputKeybindType::CONTROLLER_BUTTON}},

    {"test_up", "Test Up", InputType::TEST_UP, {InputKeybindType::KEYBOARD, InputKeybindType::CONTROLLER_BUTTON}},
    {"test_down", "Test Down", InputType::TEST_DOWN, {InputKeybindType::KEYBOARD, InputKeybindType::CONTROLLER_BUTTON}},
    {"test_enter", "Test Enter", InputType::TEST_ENTER, {InputKeybindType::KEYBOARD, InputKeybindType::CONTROLLER_BUTTON}},

    {"test", "Test", InputType::TEST, {InputKeybindType::KEYBOARD, InputKeybindType::CONTROLLER_BUTTON}},

    {"shift_down", "Shift Down", InputType::SHIFT_DOWN, {InputKeybindType::KEYBOARD, InputKeybindType::CONTROLLER_BUTTON}},
    {"shift_up", "Shift Up", InputType::SHIFT_UP, {InputKeybindType::KEYBOARD, InputKeybindType::CONTROLLER_BUTTON}},

    {"gear_one", "Shift Up", InputType::GEAR_ONE, {InputKeybindType::KEYBOARD, InputKeybindType::CONTROLLER_BUTTON}},
    {"gear_two", "Shift Up", InputType::GEAR_TWO, {InputKeybindType::KEYBOARD, InputKeybindType::CONTROLLER_BUTTON}},
    {"gear_three", "Shift Up", InputType::GEAR_THREE, {InputKeybindType::KEYBOARD, InputKeybindType::CONTROLLER_BUTTON}},
    {"gear_four", "Shift Up", InputType::GEAR_FOUR, {InputKeybindType::KEYBOARD, InputKeybindType::CONTROLLER_BUTTON}},
    {"gear_five", "Shift Up", InputType::GEAR_FIVE, {InputKeybindType::KEYBOARD, InputKeybindType::CONTROLLER_BUTTON}},
    {"gear_six", "Shift Up", InputType::GEAR_SIX, {InputKeybindType::KEYBOARD, InputKeybindType::CONTROLLER_BUTTON}},
    {"gear_neutral", "Shift Up", InputType::GEAR_NEUTRAL, {InputKeybindType::KEYBOARD, InputKeybindType::CONTROLLER_BUTTON}},

    {"card", "Card", InputType::CARD, {InputKeybindType::KEYBOARD, InputKeybindType::CONTROLLER_BUTTON}},

    {"service", "Service", InputType::SERVICE, {InputKeybindType::KEYBOARD, InputKeybindType::CONTROLLER_BUTTON}},

    {"coin", "Coin", InputType::COIN, {InputKeybindType::KEYBOARD, InputKeybindType::CONTROLLER_BUTTON}},
};

uint16_t get_analog_gas()
{
    return gas_value * float(20480);
}
uint16_t get_analog_brakes()
{
    return brakes_value * float(20480);
}
uint16_t get_analog_wheel()
{
    return wheel_value * float(UINT16_MAX);
}

bool get_view_change()
{
    return view_change_value;
}
bool get_interrupt()
{
    return interrupt_value;
}

bool get_test_up()
{
    return test_up_value;
}
bool get_test_down()
{
    return test_down_value;
}
bool get_test_enter()
{
    return test_enter_value;
}

bool get_test()
{
    return test_value;
}

int get_gear()
{
    return gear_value;
}

bool get_card()
{
    return card_value;
}

bool get_service()
{
    return service_value;
}

bool get_coin()
{
    return coin_value;
}

void on_skibidi_input(InputEvent *event)
{
    float value_float = 0.f;
    bool value_bool = false;
    switch (event->keybind->type)
    {
    case InputKeybindType::KEYBOARD:
    {
        if (event->keyboardDown)
        {
            value_float = 1.f;
            value_bool = true;
        }
        break;
    }
    case InputKeybindType::CONTROLLER_BUTTON:
    {
        if (event->controllerButtonDown)
        {
            value_float = 1.f;
            value_bool = true;
        }
        break;
    }
    case InputKeybindType::CONTROLLER_AXIS:
    {
        if (event->keybind->controllerAxisFull)
        {
            value_float = float(event->controllerAxis + SDL_JOYSTICK_AXIS_MAX) / float(SDL_JOYSTICK_AXIS_MAX * 2.f);
        }
        else
        {
            uint16_t v = event->controllerAxis;
            if (event->keybind->controllerAxisReversedHalf)
                v = UINT16_MAX - v;
            value_float = float((int16_t)v) / float(SDL_JOYSTICK_AXIS_MAX); // make it 0.f - 1.f
        }
        if (value_float < 0.f)
            value_float = 0.f;
        if (value_float > 1.f)
            value_float = 1.f;

        if (event->keybind->controllerAxisReversed)
            value_float = 1.f - value_float;
        break;
    }
    }

    bool update_wheel_lr = false;
    switch (event->input->type)
    {
    case InputType::GAS:
    {
        gas_value = value_float;
        break;
    }
    case InputType::BRAKES:
    {
        brakes_value = value_float;
        break;
    }
    case InputType::WHEEL: // can only be axis
    {
        wheel_value = value_float;
        break;
    }
    case InputType::WHEEL_LEFT: // can only be button
    {
        wheel_left = value_bool;
        update_wheel_lr = true;
        break;
    }
    case InputType::WHEEL_RIGHT: // can only be button
    {
        wheel_right = value_bool;
        update_wheel_lr = true;
        break;
    }

    case InputType::VIEW_CHANGE: // can only be button
    {
        view_change_value = value_bool;
        break;
    }
    case InputType::INTERRUPT: // can only be button
    {
        interrupt_value = value_bool;
        break;
    }

    case InputType::TEST_UP: // can only be button
    {
        test_up_value = value_bool;
        break;
    }
    case InputType::TEST_DOWN: // can only be button
    {
        test_down_value = value_bool;
        break;
    }
    case InputType::TEST_ENTER: // can only be button
    {
        test_enter_value = value_bool;
        break;
    }

    case InputType::TEST: // can only be button
    {
        if (event->keybind->type == InputKeybindType::CONTROLLER_BUTTON)
        {
            keyTestPressed++;
            if(keyTestPressed % 4 == 0 && keyTestPressed != 0)
            {
                test_value = !test_value;
                keyTestPressed = 0;
            }
        }
        else if(value_bool)
        {
            test_value = !test_value;
        }
        break;
    }

    case InputType::SHIFT_DOWN: // can only be button
    {
        if(value_bool)
        {
            if ((!keyShiftPressed && event->keybind->type == InputKeybindType::CONTROLLER_BUTTON) ||
                event->keybind->type != InputKeybindType::CONTROLLER_BUTTON)
            {
                keyShiftPressed = true;
                gear_value--;
                if (gear_value < 0)
                    gear_value = 0;
            }
        }
        break;
    }
    case InputType::SHIFT_UP: // can only be button
    {
        if(value_bool)
        {
            if ((!keyShiftPressed && event->keybind->type == InputKeybindType::CONTROLLER_BUTTON) ||
                event->keybind->type != InputKeybindType::CONTROLLER_BUTTON)
            {
                keyShiftPressed = true;
                gear_value++;
                if (gear_value > 6)
                    gear_value = 6;
            }
        }
        break;
    }

    case InputType::GEAR_ONE:
    case InputType::GEAR_TWO:
    case InputType::GEAR_THREE:
    case InputType::GEAR_FOUR:
    case InputType::GEAR_FIVE:
    case InputType::GEAR_SIX:
    case InputType::GEAR_NEUTRAL: // can only be button
    {
        if(value_bool)
        {
            if(event->input->type == InputType::GEAR_ONE)
                gear_value = 1;
            else if(event->input->type == InputType::GEAR_TWO)
                gear_value = 2;
            else if(event->input->type == InputType::GEAR_THREE)
                gear_value = 3;
            else if(event->input->type == InputType::GEAR_FOUR)
                gear_value = 4;
            else if(event->input->type == InputType::GEAR_FIVE)
                gear_value = 5;
            else if(event->input->type == InputType::GEAR_SIX)
                gear_value = 6;
            else if(event->input->type == InputType::GEAR_NEUTRAL)
                gear_value = 0;

            std::thread([gear_ptr = &gear_value]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                *gear_ptr = 0;
            }).detach();
        }
        break;
    }

    case InputType::CARD:
    {
        card_value = value_bool;
        break;
    }

    case InputType::SERVICE:
    {
        service_value = value_bool;
        break;
    }

    case InputType::COIN:
    {
        coin_value = value_bool;
        break;
    }
    }

    if (update_wheel_lr)
    {
        wheel_value = 0.5f;

        if (wheel_left)
            wheel_value -= 0.5f;
        if (wheel_right)
            wheel_value += 0.5f;
    }
}

Input *adding_keybind = NULL;

bool inputSupportsKeybind(Input *input, InputKeybindType type)
{
    for (size_t i = 0; i < input->keybindsTypes.size(); i++)
        if (input->keybindsTypes[i] == type)
            return true;
    return false;
}

bool inputEquals(const InputKeybind &a, const InputKeybind &b)
{
    if (a.type != b.type)
        return false;
    // TODO: add joystick support
    switch (a.type)
    {
    case InputKeybindType::KEYBOARD:
        return a.keyboard == b.keyboard;
    case InputKeybindType::CONTROLLER_BUTTON:
        return a.controllerButton == b.controllerButton;
    case InputKeybindType::CONTROLLER_AXIS:
        return a.controllerAxis == b.controllerAxis;
    }
    return false;
}

void input_sdl(const SDL_Event &event)
{
    switch (event.type)
    {
    case SDL_KEYUP:
    case SDL_KEYDOWN:
    {
        if (adding_keybind)
        {
            if (!inputSupportsKeybind(adding_keybind, InputKeybindType::KEYBOARD))
                break;
            // check if keybinds exists
            const InputKeybind newBind = {InputKeybindType::KEYBOARD, event.key.keysym.sym};
            bool keybindExists = false;
            for (const InputKeybind &bind : adding_keybind->keybinds)
            {
                if (inputEquals(bind, newBind))
                {
                    keybindExists = true;
                    break;
                }
            }
            if (!keybindExists)
            {
                adding_keybind->keybinds.push_back(newBind);
                adding_keybind = NULL;
            }
            break;
        }

        bool down = event.type == SDL_KEYDOWN;

        for (size_t i = 0; i < inputs.size(); i++)
        {
            for (size_t j = 0; j < inputs[i].keybinds.size(); j++)
            {
                if (inputs[i].keybinds[j].type != InputKeybindType::KEYBOARD)
                    continue;
                if (inputs[i].keybinds[j].keyboard != event.key.keysym.sym)
                    continue;

                InputEvent e;
                e.input = &inputs[i];
                e.keybind = &inputs[i].keybinds[j];
                e.keyboardDown = down;
                on_skibidi_input(&e);
            }
        }

        break;
    }
    case SDL_CONTROLLERAXISMOTION:
    {
        if (adding_keybind)
        {
            if (!inputSupportsKeybind(adding_keybind, InputKeybindType::CONTROLLER_AXIS))
                break;

            // check if keybinds exists
            InputKeybind newBind = {InputKeybindType::CONTROLLER_AXIS, event.caxis.axis};
            if (adding_keybind->type == InputType::WHEEL) // set to true by default for wheel
                newBind.controllerAxisFull = true;

            bool keybindExists = false;
            for (const InputKeybind &bind : adding_keybind->keybinds)
            {
                if (inputEquals(bind, newBind))
                {
                    keybindExists = true;
                    break;
                }
            }
            if (!keybindExists)
            {
                adding_keybind->keybinds.push_back(newBind);
                adding_keybind = NULL;
            }
            break;
        }

        for (size_t i = 0; i < inputs.size(); i++)
        {
            for (size_t j = 0; j < inputs[i].keybinds.size(); j++)
            {
                if (inputs[i].keybinds[j].type != InputKeybindType::CONTROLLER_AXIS)
                    continue;
                if (inputs[i].keybinds[j].controllerAxis != event.caxis.axis)
                    continue;

                InputEvent e;
                e.input = &inputs[i];
                e.keybind = &inputs[i].keybinds[j];
                e.controllerAxis = event.caxis.value;
                on_skibidi_input(&e);
            }
        }

        break;
    }
    case SDL_CONTROLLERBUTTONUP:
    case SDL_CONTROLLERBUTTONDOWN:
    {
        if (adding_keybind)
        {
            if (!inputSupportsKeybind(adding_keybind, InputKeybindType::CONTROLLER_BUTTON))
                break;

            // check if keybinds exists
            const InputKeybind newBind = {InputKeybindType::CONTROLLER_BUTTON, event.cbutton.button};
            bool keybindExists = false;
            for (const InputKeybind &bind : adding_keybind->keybinds)
            {
                if (inputEquals(bind, newBind))
                {
                    keybindExists = true;
                    break;
                }
            }
            if (!keybindExists)
            {
                adding_keybind->keybinds.push_back(newBind);
                adding_keybind = NULL;
            }
            break;
        }

        bool down = event.type == SDL_CONTROLLERBUTTONDOWN;
        for (size_t i = 0; i < inputs.size(); i++)
        {
            for (size_t j = 0; j < inputs[i].keybinds.size(); j++)
            {
                if (inputs[i].keybinds[j].type != InputKeybindType::CONTROLLER_BUTTON)
                    continue;
                if (inputs[i].keybinds[j].controllerButton != event.cbutton.button)
                    continue;

                InputEvent e;
                e.input = &inputs[i];
                e.keybind = &inputs[i].keybinds[j];
                e.controllerButtonDown = down;
                on_skibidi_input(&e);
            }
        }

        if(event.type == SDL_CONTROLLERBUTTONUP)
            keyShiftPressed = false;
        
        break;
    }
    case SDL_CONTROLLERDEVICEADDED:
    {
        if (!SDL_IsGameController(event.cdevice.which))
            break;
        SDL_GameControllerOpen(event.cdevice.which);
        break;
    }
    case SDL_CONTROLLERDEVICEREMOVED:
    {
        auto controller = SDL_GameControllerFromInstanceID(event.cdevice.which);
        if (!controller)
            break;
        SDL_GameControllerClose(controller);
        break;
    }
    }
}

Input *input_ext_get_adding_keybind()
{
    return adding_keybind;
}
void input_ext_set_adding_keybind(Input *input)
{
    adding_keybind = input;
}

void input_config_read(toml::table config)
{
    for (size_t i = 0; i < inputs.size(); i++)
    {
        inputs[i].keybinds.clear();

        if (!config[inputs[i].name].is_array())
            continue;
        auto inputConfig = *config[inputs[i].name].as_array();
        for (size_t j = 0; j < inputConfig.size(); j++)
        {
            if (!inputConfig[j].is_table())
                continue;
            auto keyConfig = *inputConfig[j].as_table();

            if (keyConfig["keyboard"].is_integer())
            {
                const InputKeybind newBind = {InputKeybindType::KEYBOARD, (SDL_Keycode)keyConfig["keyboard"].as_integer()->get()};
                bool keybindExists = false;
                for (const InputKeybind &bind : inputs[i].keybinds)
                {
                    if (inputEquals(bind, newBind))
                    {
                        keybindExists = true;
                        break;
                    }
                }
                if (!keybindExists)
                    inputs[i].keybinds.push_back(newBind);
            }

            if (keyConfig["controller_axis"].is_integer())
            {
                InputKeybind newBind = {InputKeybindType::CONTROLLER_AXIS, (SDL_GameControllerAxis)keyConfig["controller_axis"].as_integer()->get()};
                if (keyConfig["controller_axis_full"].is_boolean() && keyConfig["controller_axis_full"].as_boolean()->get())
                    newBind.controllerAxisFull = true;
                if (keyConfig["controller_axis_reversed"].is_boolean() && keyConfig["controller_axis_reversed"].as_boolean()->get())
                    newBind.controllerAxisReversed = true;
                if (keyConfig["controller_axis_reversed_half"].is_boolean() && keyConfig["controller_axis_reversed_half"].as_boolean()->get())
                    newBind.controllerAxisReversedHalf = true;

                bool keybindExists = false;
                for (const InputKeybind &bind : inputs[i].keybinds)
                {
                    if (inputEquals(bind, newBind))
                    {
                        keybindExists = true;
                        break;
                    }
                }
                if (!keybindExists)
                    inputs[i].keybinds.push_back(newBind);
            }

            if (keyConfig["controller_button"].is_integer())
            {
                const InputKeybind newBind = {InputKeybindType::CONTROLLER_BUTTON, (SDL_GameControllerButton)keyConfig["controller_button"].as_integer()->get()};

                bool keybindExists = false;
                for (const InputKeybind &bind : inputs[i].keybinds)
                {
                    if (inputEquals(bind, newBind))
                    {
                        keybindExists = true;
                        break;
                    }
                }
                if (!keybindExists)
                    inputs[i].keybinds.push_back(newBind);
            }
        }
    }
}
toml::table input_config_write()
{
    toml::table config;

    for (size_t i = 0; i < inputs.size(); i++)
    {
        toml::array inputConfig;

        for (size_t j = 0; j < inputs[i].keybinds.size(); j++)
        {
            toml::table keyConfig;
            auto keybind = inputs[i].keybinds[j];

            switch (keybind.type)
            {
            case InputKeybindType::KEYBOARD:
            {
                keyConfig.insert_or_assign("keyboard", (int64_t)keybind.keyboard);
                break;
            }
            case InputKeybindType::CONTROLLER_AXIS:
            {
                keyConfig.insert_or_assign("controller_axis", (int64_t)keybind.controllerAxis);
                if (keybind.controllerAxisFull)
                    keyConfig.insert_or_assign("controller_axis_full", true);
                if (keybind.controllerAxisReversed)
                    keyConfig.insert_or_assign("controller_axis_reversed", true);
                if (keybind.controllerAxisReversedHalf)
                    keyConfig.insert_or_assign("controller_axis_reversed_half", true);
                break;
            }
            case InputKeybindType::CONTROLLER_BUTTON:
            {
                keyConfig.insert_or_assign("controller_button", (int64_t)keybind.controllerButton);
                break;
            }
            }

            inputConfig.push_back(keyConfig);
        }

        config.insert_or_assign(inputs[i].name, inputConfig);
    }

    return config;
}