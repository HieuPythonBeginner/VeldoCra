/**
 * @file main.cpp
 * @brief Veldanava Compiler - Main Entry Point
 * @author Dr. Bright
 * 
 * The official Veldanava compiler (veldanc)
 * Usage:
 *   veldanc build <file.veldan>  - Compile to native binary
 *   veldanc run <file.veldan>    - Run via VM
 *   veldanc check <file.veldan>  - Static analysis only
 */

#include <iostream>
#include <string_view>
#include <vector>
#include "driver/driver.h"

using namespace veldanava;

/**
 * @brief Print usage information
 */
void print_usage(std::string_view program_name) {
    std::cout << "Veldanava Compiler v" << Driver::version() << "\n";
    std::cout << "Usage:\n";
    std::cout << "  " << program_name << " build <file.veldan> [output]\n";
    std::cout << "  " << program_name << " run <file.veldan>\n";
    std::cout << "  " << program_name << " check <file.veldan>\n";
    std::cout << "  " << program_name << " selfhost\n";
    std::cout << "\nCommands:\n";
    std::cout << "  build    - Compile .veldan file to native binary\n";
    std::cout << "  run      - Execute .veldan file via VM\n";
    std::cout << "  check    - Perform static analysis only\n";
    std::cout << "  selfhost - Test if Veldanava can compile its own source files\n";
}

/**
 * @brief Parse command line arguments
 */
bool parse_args(int argc, char** argv, Command& cmd, 
                std::string_view& input, std::string_view& output) {
    if (argc < 2) {
        return false;
    }
    
    std::string_view command = argv[1];
    
    if (command == "selfhost") {
        cmd = Command::SelfHost;
        return true;
    }
    
    if (argc < 3) {
        return false;
    }
    
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
    Command cmd;
    std::string_view input;
    std::string_view output;
    
    if (!parse_args(argc, argv, cmd, input, output)) {
        print_usage(argv[0]);
        return 1;
    }
    
    Driver driver;
    auto result = driver.execute(cmd, input, output);
    
    if (!result.success) {
        std::cerr << "Error: " << result.error << "\n";
        return 1;
    }
    
    return result.value;
}