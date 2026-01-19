#include <iostream>
#include <string>
#include <memory>
#include "server.h"

int main(int argc, char* argv[]) {
    std::string config_file;
    
    if (argc > 1) {
        config_file = argv[1];
    }
    
    try {
        ForteServer server("localhost", 61499);
        
        if (!config_file.empty()) {
            std::cout << "Loading the configuration from " << config_file << std::endl;
            server.load_boot_file(config_file);
        }
        
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}