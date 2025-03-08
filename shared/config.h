#pragma once
#include <toml.hpp>

struct Config {
    bool terminal;
    bool terminalProxy;
    
    bool bana;
    bool autoScan;
    std::string accessCode;
    std::string chipId;

    bool input;
    bool inputBackground;

    bool touch;
    
    bool yac;

    std::string ipAddress;
    
    void load();
    void save();
};

extern Config config;