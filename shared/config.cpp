#include "config.h"
#include <toml.hpp>
#include "input.h"

void Config::load()
{
    {
        toml::table config;
        try
        {
            config = toml::parse_file("config.toml");
        }
        catch (const toml::parse_error &err)
        {
            printf("Error parsing config.toml: %s\n", err.what());
        }

        if (config["terminal"].is_boolean())
        {
            this->terminal = config["terminal"].as_boolean()->get();
        }
        else
        {
            this->terminal = false;
        }

        if (config["terminal_proxy"].is_boolean())
        {
            this->terminalProxy = config["terminal_proxy"].as_boolean()->get();
        }
        else
        {
            this->terminalProxy = false;
        }

        if (config["bana"].is_table())
        {
            auto banaConfig = config["bana"];

            if (banaConfig["enabled"].is_boolean())
            {
                this->bana = banaConfig["enabled"].as_boolean()->get();
            }
            else
            {
                this->bana = true;
            }

            if (banaConfig["auto_scan"].is_boolean())
            {
                this->autoScan = banaConfig["auto_scan"].as_boolean()->get();
            }
            else
            {
                this->autoScan = false;
            }

            // assume the user isnt using a long banapass access code / chipid
            this->accessCode = "30764352518498791337";
            this->chipId = "7F5C9744F111111143262C3300040610";
        }
        else
        {
            this->accessCode = "30764352518498791337";
            this->chipId = "7F5C9744F111111143262C3300040610";
        }

        if (config["input"].is_table())
        {
            auto inputConfig = config["input"];

            if (inputConfig["enabled"].is_boolean())
            {
                this->input = inputConfig["enabled"].as_boolean()->get();
            }
            else
            {
                this->input = true;
            }

            if (inputConfig["background"].is_boolean())
            {
                this->inputBackground = inputConfig["background"].as_boolean()->get();
            }
            else
            {
                this->inputBackground = false;
            }
        }

        if (config["touch"].is_table())
        {
            auto touchConfig = config["touch"];

            if (touchConfig["enabled"].is_boolean())
            {
                this->touch = touchConfig["enabled"].as_boolean()->get();
            }
            else
            {
                this->touch = true;
            }
        }

        if (config["yac"].is_table())
        {
            auto yacConfig = config["yac"];

            if (yacConfig["enabled"].is_boolean())
            {
                this->yac = yacConfig["enabled"].as_boolean()->get();
            }
            else
            {
                this->yac = false;
            }
        }
        else
        {
            this->yac = false;
        }

        if (config["network"].is_table())
        {
            auto networkConfig = config["network"];

            if (networkConfig["ip_address"].is_string())
            {
                this->ipAddress = networkConfig["ip_address"].as_string()->get();
            }
            else
            {
                this->ipAddress = "mucha.local";
            }
        }
        else
        {
            this->ipAddress = "mucha.local";
        }
    }

    {
        toml::table config;
        try
        {
            config = toml::parse_file("input.toml");
        }
        catch (const toml::parse_error &err)
        {
            printf("Error parsing input.toml: %s\n", err.what());
        }

        input_config_read(config);
    }
}

void Config::save()
{
    {
        toml::table config;

        config.insert_or_assign("terminal", this->terminal);
        config.insert_or_assign("terminal_proxy", this->terminalProxy);

        toml::table banaConfig;
        banaConfig.insert_or_assign("enabled", this->bana);
        banaConfig.insert_or_assign("auto_scan", this->autoScan);
        config.insert_or_assign("bana", banaConfig);

        toml::table inputConfig;
        inputConfig.insert_or_assign("enabled", this->input);
        inputConfig.insert_or_assign("background", this->inputBackground);
        config.insert_or_assign("input", inputConfig);

        toml::table touchConfig;
        touchConfig.insert_or_assign("enabled", this->touch);
        config.insert_or_assign("input", touchConfig);
        
        toml::table yacConfig;
        yacConfig.insert_or_assign("enabled", this->yac);
        config.insert_or_assign("yac", yacConfig);

        toml::table networkConfig;
        networkConfig.insert_or_assign("ip_address", this->ipAddress);
        config.insert_or_assign("network", networkConfig);

        std::ofstream file;
        file.open("config.toml");
        if (file.is_open())
        {
            file << config;
            file.close();
        }
        else
        {
            printf("Error writing config.toml\n");
        }
    }
    {
        toml::table config = input_config_write();

        std::ofstream file;
        file.open("input.toml");
        if (file.is_open())
        {
            file << config;
            file.close();
        }
        else
        {
            printf("Error writing input.toml\n");
        }
    }
}

Config config;