/**
 * @file main.cpp
 * @brief VeldoCra Compiler - Main Entry Point
 * @author Dr. Bright
 * 
 * The official VeldoCra compiler (velc)
 * Usage:
 *   velc build <file.vel>  - Compile to native binary
 *   velc run <file.vel>    - Run via VM
 *   velc check <file.vel>  - Static analysis only
 */

#include <iostream>
#include <string_view>
#include <vector>
#include "driver/driver.h"

using namespace veldocra;

/**
 * @brief Print usage information
 */
void print_usage(std::string_view program_name) {
    std::cout << "VeldoCra Compiler v" << Driver::version() << "\n";
    std::cout << "Usage:\n";
    std::cout << "  " << program_name << " build <file.vel> [output]\n";
    std::cout << "  " << program_name << " run <file.vel>\n";
    std::cout << "  " << program_name << " check <file.vel>\n";
    std::cout << "\nCommands:\n";
    std::cout << "  build  - Compile .vel file to native binary\n";
    std::cout << "  run    - Execute .vel file via VM\n";
    std::cout << "  check  - Perform static analysis only\n";
}

/**
 * @brief Parse command line arguments
 */
bool parse_args(int argc, char** argv, Command& cmd, 
                std::string_view& input, std::string_view& output) {
    if (argc < 3) {
        return false;
    }
    
    std::string_view command = argv[1];
    input = argv[2];
    
    if (command == "build") {
        cmd = Command::Build;
        if (argc >= 4) {
            output = argv[3];
        }
        return true;
    } else if (command == "run") {
        cmd = Command::Run;
        return true;
    } else if (command == "check") {
        cmd = Command::Check;
        return true;
    }
    
    return false;
}

/**
 * @brief Main entry point
 */
int main(int argc, char** argv) {
    // Parse arguments
    Command cmd;
    std::string_view input;
    std::string_view output;
    
    if (!parse_args(argc, argv, cmd, input, output)) {
        print_usage(argv[0]);
        return 1;
    }
    
    // Create driver and execute
    Driver driver;
    auto result = driver.execute(cmd, input, output);
    
    if (!result.success) {
        std::cerr << "Error: " << result.error << "\n";
        return 1;
    }
    
    return result.value;
}

