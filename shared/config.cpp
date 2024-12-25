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

            // assume the user isnt using a long banapass access code / chipid
            if (banaConfig["access_code"].is_string())
            {
                strcpy(this->accessCode, banaConfig["access_code"].as_string()->get().c_str());
            }
            else
            {
                strcpy(this->accessCode, "30730273167899430416");
            }
            if (banaConfig["chip_id"].is_string())
            {
                strcpy(this->chipId, banaConfig["chip_id"].as_string()->get().c_str());
            }
            else
            {
                strcpy(this->chipId, "00000000000000000000000000000000");
            }
        }
        else
        {
            strcpy(this->accessCode, "30730273167899430416");
            strcpy(this->chipId, "00000000000000000000000000000000");
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
        banaConfig.insert_or_assign("access_code", this->accessCode);
        banaConfig.insert_or_assign("chip_id", this->chipId);
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