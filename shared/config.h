#pragma once
#include <toml.hpp>

struct Config {
    bool terminal;
    bool terminalProxy;
    
    bool bana;
    char accessCode[24];
    char chipId[36];

    bool input;
    bool inputBackground;

    bool touch;
    
    bool yac;
    
    void load();
    void save();
};

extern Config config;