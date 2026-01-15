#ifndef CONFIG_HPP
#define CONFIG_HPP
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
class Config {
public:
    float color_red = 1.0f;
    float color_green = 1.0f;
    float color_blue = 1.0f;
    float simulation_speed = 1.0f;
    float thickness = 1.0f;
    float bloom_power = 1.0f;
    float bloom_distance = 1.0f;
    float bloom_flicker_intensity = 0.0f;
    float cycle_speed = 1.0f;
    float cycle_speed_randomness = 1.0f;
    float hdr_multiplier = 3.0f;
    float edge_smoothing = 0.5f;
    float antialiasing_min = 0.7f; 
    float antialiasing_max = 1.0f;
    
    // Load configuration from INI file
    void load(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cout << "Config file not found, using defaults\n";
            return;
        }
        std::string line;
        while (std::getline(file, line)) {
            // Skip empty lines and comments
            if (line.empty() || line[0] == '#' || line[0] == ';') {
                continue;
            }
            // Find the '=' separator
            size_t pos = line.find('=');
            if (pos == std::string::npos) {
                continue;
            }
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            // Trim whitespace
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            // Parse the value
            float val = std::stof(value);
            // Set the appropriate config value
            if (key == "color_red") {
                color_red = val;
            } else if (key == "color_green") {
                color_green = val;
            } else if (key == "color_blue") {
                color_blue = val;
            } else if (key == "simulation_speed") {
                simulation_speed = val;
            } else if (key == "thickness") {
                thickness = val;
            } else if (key == "bloom_power") {
                bloom_power = val;
            } else if (key == "bloom_distance") {
                bloom_distance = val;
            } else if (key == "bloom_flicker_intensity") {
                bloom_flicker_intensity = val;
            } else if (key == "cycle_speed") {
                cycle_speed = val;
            } else if (key == "cycle_speed_randomness") {
                cycle_speed_randomness = val;
            } else if (key == "hdr_multiplier") {
                hdr_multiplier = val;
            } else if (key == "edge_smoothing") {
                edge_smoothing = val;
            } else if (key == "antialiasing_min") {
                antialiasing_min = val;
            } else if (key == "antialiasing_max") {
                antialiasing_max = val;
            }
        }
        file.close();
        std::cout << "Config loaded from " << filename << "\n";
    }
    
    // Save configuration to INI file
    void save(const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to save config file\n";
            return;
        }
        file << "# Tron Wallpaper Configuration\n";
        file << "color_red=" << color_red << "\n";
        file << "color_green=" << color_green << "\n";
        file << "color_blue=" << color_blue << "\n";
        file << "simulation_speed=" << simulation_speed << "\n";
        file << "thickness=" << thickness << "\n";
        file << "bloom_power=" << bloom_power << "\n";
        file << "bloom_distance=" << bloom_distance << "\n";
        file << "bloom_flicker_intensity=" << bloom_flicker_intensity << "\n";
        file << "cycle_speed=" << cycle_speed << "\n";
        file << "cycle_speed_randomness=" << cycle_speed_randomness << "\n";
        file << "hdr_multiplier=" << hdr_multiplier << "\n";
        file << "edge_smoothing=" << edge_smoothing << "\n";
        file << "antialiasing_min=" << antialiasing_min << "\n";
        file << "antialiasing_max=" << antialiasing_max << "\n";
        file.close();
        std::cout << "Config saved to " << filename << "\n";
    }
};
#endif // CONFIG_HPP